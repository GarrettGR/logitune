#include "EditorModel.h"
#include "DeviceRegistry.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>

namespace logitune {

EditorModel::EditorModel(DeviceRegistry *registry, bool editing, QObject *parent)
    : QObject(parent), m_registry(registry), m_editing(editing) {}

bool EditorModel::canUndo() const {
    auto it = m_undoStacks.find(m_activeDevicePath);
    return it != m_undoStacks.end() && !it->isEmpty();
}

bool EditorModel::canRedo() const {
    auto it = m_redoStacks.find(m_activeDevicePath);
    return it != m_redoStacks.end() && !it->isEmpty();
}

void EditorModel::setActiveDevicePath(const QString &path) {
    if (path == m_activeDevicePath) return;
    m_activeDevicePath = path;
    emit activeDevicePathChanged();
    emit dirtyChanged();
    emit undoStateChanged();
}

void EditorModel::ensurePending(const QString &path) {
    if (m_pendingEdits.contains(path)) return;
    QFile f(path + QStringLiteral("/descriptor.json"));
    if (!f.open(QIODevice::ReadOnly)) return;
    m_pendingEdits[path] = QJsonDocument::fromJson(f.readAll()).object();
}

void EditorModel::pushCommand(EditCommand cmd) {
    m_undoStacks[m_activeDevicePath].push(std::move(cmd));
    m_redoStacks[m_activeDevicePath].clear();
    m_dirty.insert(m_activeDevicePath);
    emit dirtyChanged();
    emit undoStateChanged();
}

void EditorModel::updateSlotPosition(int idx, double xPct, double yPct) {
    if (m_activeDevicePath.isEmpty()) return;
    ensurePending(m_activeDevicePath);
    QJsonObject &obj = m_pendingEdits[m_activeDevicePath];
    QJsonArray slotsArr = obj.value(QStringLiteral("easySwitchSlots")).toArray();
    if (idx < 0 || idx >= slotsArr.size()) return;

    EditCommand cmd;
    cmd.kind = EditCommand::SlotMove;
    cmd.index = idx;
    cmd.before = slotsArr[idx];

    QJsonObject slot = slotsArr[idx].toObject();
    slot[QStringLiteral("xPct")] = xPct;
    slot[QStringLiteral("yPct")] = yPct;
    slotsArr[idx] = slot;
    obj[QStringLiteral("easySwitchSlots")] = slotsArr;
    cmd.after = slotsArr[idx];

    pushCommand(std::move(cmd));
}

void EditorModel::updateHotspot(int idx, double xPct, double yPct,
                                 const QString &side, double labelOffsetYPct) {
    if (m_activeDevicePath.isEmpty()) return;
    ensurePending(m_activeDevicePath);
    QJsonObject &obj = m_pendingEdits[m_activeDevicePath];
    QJsonObject hotspots = obj.value(QStringLiteral("hotspots")).toObject();
    QJsonArray buttons = hotspots.value(QStringLiteral("buttons")).toArray();
    if (idx < 0 || idx >= buttons.size()) return;

    EditCommand cmd;
    cmd.kind = EditCommand::HotspotMove;
    cmd.index = idx;
    cmd.before = buttons[idx];

    QJsonObject hs = buttons[idx].toObject();
    hs[QStringLiteral("xPct")] = xPct;
    hs[QStringLiteral("yPct")] = yPct;
    hs[QStringLiteral("side")] = side;
    hs[QStringLiteral("labelOffsetYPct")] = labelOffsetYPct;
    buttons[idx] = hs;
    hotspots[QStringLiteral("buttons")] = buttons;
    obj[QStringLiteral("hotspots")] = hotspots;
    cmd.after = buttons[idx];

    pushCommand(std::move(cmd));
}

void EditorModel::updateText(const QString &field, int index, const QString &value) {
    if (m_activeDevicePath.isEmpty()) return;
    ensurePending(m_activeDevicePath);
    QJsonObject &obj = m_pendingEdits[m_activeDevicePath];

    EditCommand cmd;
    cmd.kind = EditCommand::TextEdit;
    cmd.role = field;
    cmd.index = index;

    if (field == QStringLiteral("deviceName")) {
        cmd.before = obj.value(QStringLiteral("name"));
        cmd.after = value;
        obj[QStringLiteral("name")] = value;
    } else if (field == QStringLiteral("controlDisplayName")) {
        QJsonArray controls = obj.value(QStringLiteral("controls")).toArray();
        if (index < 0 || index >= controls.size()) return;
        QJsonObject c = controls[index].toObject();
        cmd.before = c.value(QStringLiteral("displayName"));
        c[QStringLiteral("displayName")] = value;
        controls[index] = c;
        obj[QStringLiteral("controls")] = controls;
        cmd.after = value;
    } else if (field == QStringLiteral("slotLabel")) {
        QJsonArray slotsArr = obj.value(QStringLiteral("easySwitchSlots")).toArray();
        if (index < 0 || index >= slotsArr.size()) return;
        QJsonObject s = slotsArr[index].toObject();
        cmd.before = s.value(QStringLiteral("label"));
        s[QStringLiteral("label")] = value;
        slotsArr[index] = s;
        obj[QStringLiteral("easySwitchSlots")] = slotsArr;
        cmd.after = value;
    } else {
        return;
    }
    pushCommand(std::move(cmd));
}

void EditorModel::applyCommand(const EditCommand &cmd, bool reverse) {
    QJsonObject &obj = m_pendingEdits[m_activeDevicePath];
    const QJsonValue &target = reverse ? cmd.before : cmd.after;
    if (cmd.kind == EditCommand::SlotMove) {
        QJsonArray slotsArr = obj.value(QStringLiteral("easySwitchSlots")).toArray();
        slotsArr[cmd.index] = target;
        obj[QStringLiteral("easySwitchSlots")] = slotsArr;
    } else if (cmd.kind == EditCommand::HotspotMove) {
        QJsonObject hotspots = obj.value(QStringLiteral("hotspots")).toObject();
        QJsonArray buttons = hotspots.value(QStringLiteral("buttons")).toArray();
        buttons[cmd.index] = target;
        hotspots[QStringLiteral("buttons")] = buttons;
        obj[QStringLiteral("hotspots")] = hotspots;
    } else if (cmd.kind == EditCommand::ImageReplace) {
        QJsonObject images = obj.value(QStringLiteral("images")).toObject();
        images[cmd.role] = target.toString();
        obj[QStringLiteral("images")] = images;
    } else if (cmd.kind == EditCommand::TextEdit) {
        if (cmd.role == QStringLiteral("deviceName")) {
            obj[QStringLiteral("name")] = target;
        } else if (cmd.role == QStringLiteral("controlDisplayName")) {
            QJsonArray controls = obj.value(QStringLiteral("controls")).toArray();
            QJsonObject c = controls[cmd.index].toObject();
            c[QStringLiteral("displayName")] = target;
            controls[cmd.index] = c;
            obj[QStringLiteral("controls")] = controls;
        } else if (cmd.role == QStringLiteral("slotLabel")) {
            QJsonArray slotsArr = obj.value(QStringLiteral("easySwitchSlots")).toArray();
            QJsonObject s = slotsArr[cmd.index].toObject();
            s[QStringLiteral("label")] = target;
            slotsArr[cmd.index] = s;
            obj[QStringLiteral("easySwitchSlots")] = slotsArr;
        }
    }
}

void EditorModel::undo() {
    auto &stack = m_undoStacks[m_activeDevicePath];
    if (stack.isEmpty()) return;
    EditCommand cmd = stack.pop();
    applyCommand(cmd, true);
    m_redoStacks[m_activeDevicePath].push(cmd);

    if (stack.isEmpty()) m_dirty.remove(m_activeDevicePath);
    emit dirtyChanged();
    emit undoStateChanged();
}

void EditorModel::redo() {
    auto &stack = m_redoStacks[m_activeDevicePath];
    if (stack.isEmpty()) return;
    EditCommand cmd = stack.pop();
    applyCommand(cmd, false);
    m_undoStacks[m_activeDevicePath].push(cmd);
    m_dirty.insert(m_activeDevicePath);
    emit dirtyChanged();
    emit undoStateChanged();
}

} // namespace logitune
