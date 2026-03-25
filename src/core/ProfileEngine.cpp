#include "ProfileEngine.h"
#include <QDir>
#include <QFileInfo>

namespace logitune {

// ---------------------------------------------------------------------------
// ButtonAction
// ---------------------------------------------------------------------------

ButtonAction ButtonAction::parse(const QString &str)
{
    if (str.isEmpty() || str == "default")
        return {Default, {}};

    if (str == "gesture-trigger")
        return {GestureTrigger, {}};

    // Prefixed forms: "type:payload"
    const int colon = str.indexOf(':');
    if (colon == -1)
        return {Default, {}};

    const QString prefix  = str.left(colon);
    const QString payload = str.mid(colon + 1);

    if (prefix == "keystroke")  return {Keystroke,  payload};
    if (prefix == "media")      return {Media,       payload};
    if (prefix == "dbus")       return {DBus,        payload};
    if (prefix == "app-launch") return {AppLaunch,   payload};

    // Unknown prefix — treat as default
    return {Default, {}};
}

QString ButtonAction::serialize() const
{
    switch (type) {
    case Default:       return "default";
    case GestureTrigger: return "gesture-trigger";
    case Keystroke:     return "keystroke:" + payload;
    case Media:         return "media:" + payload;
    case DBus:          return "dbus:" + payload;
    case AppLaunch:     return "app-launch:" + payload;
    }
    return "default";
}

// ---------------------------------------------------------------------------
// ProfileEngine — static helpers
// ---------------------------------------------------------------------------

Profile ProfileEngine::loadProfile(const QString &path)
{
    QSettings s(path, QSettings::IniFormat);
    Profile p;

    p.version             = s.value("General/version", 1).toInt();
    p.name                = s.value("General/name").toString();
    p.icon                = s.value("General/icon").toString();

    p.dpi                 = s.value("DPI/value", 1000).toInt();

    p.smartShiftEnabled   = s.value("SmartShift/enabled", true).toBool();
    p.smartShiftThreshold = s.value("SmartShift/threshold", 128).toInt();

    const QString scrollMode = s.value("Scroll/mode", "ratchet").toString();
    p.smoothScrolling     = (scrollMode == "smooth");
    p.scrollDirection     = s.value("Scroll/direction", "standard").toString();
    p.hiResScroll         = s.value("Scroll/hires", true).toBool();

    s.beginGroup("Buttons");
    const QStringList buttonKeys = s.childKeys();
    for (const QString &key : buttonKeys) {
        bool ok = false;
        int idx = key.toInt(&ok);
        if (ok && idx >= 0 && idx < static_cast<int>(p.buttons.size()))
            p.buttons[static_cast<std::size_t>(idx)] = ButtonAction::parse(s.value(key).toString());
    }
    s.endGroup();

    s.beginGroup("Gestures");
    const QStringList gestureKeys = s.childKeys();
    for (const QString &key : gestureKeys)
        p.gestures[key] = ButtonAction::parse(s.value(key).toString());
    s.endGroup();

    return p;
}

void ProfileEngine::saveProfile(const QString &path, const Profile &profile)
{
    QSettings s(path, QSettings::IniFormat);

    s.setValue("General/version", profile.version);
    s.setValue("General/name",    profile.name);
    s.setValue("General/icon",    profile.icon);

    s.setValue("DPI/value", profile.dpi);

    s.setValue("SmartShift/enabled",   profile.smartShiftEnabled);
    s.setValue("SmartShift/threshold", profile.smartShiftThreshold);

    s.setValue("Scroll/mode",      profile.smoothScrolling ? "smooth" : "ratchet");
    s.setValue("Scroll/direction", profile.scrollDirection);
    s.setValue("Scroll/hires",     profile.hiResScroll);

    s.beginGroup("Buttons");
    for (std::size_t i = 0; i < profile.buttons.size(); ++i)
        s.setValue(QString::number(static_cast<int>(i)), profile.buttons[i].serialize());
    s.endGroup();

    s.beginGroup("Gestures");
    for (const auto &[key, action] : profile.gestures)
        s.setValue(key, action.serialize());
    s.endGroup();

    s.sync();
}

QMap<QString, QString> ProfileEngine::loadAppBindings(const QString &path)
{
    QSettings s(path, QSettings::IniFormat);
    QMap<QString, QString> bindings;

    s.beginGroup("Bindings");
    const QStringList keys = s.childKeys();
    for (const QString &key : keys)
        bindings[key] = s.value(key).toString();
    s.endGroup();

    return bindings;
}

void ProfileEngine::saveAppBindings(const QString &path, const QMap<QString, QString> &bindings)
{
    QSettings s(path, QSettings::IniFormat);

    s.remove("Bindings");
    s.beginGroup("Bindings");
    for (auto it = bindings.cbegin(); it != bindings.cend(); ++it)
        s.setValue(it.key(), it.value());
    s.endGroup();

    s.sync();
}

ProfileDelta ProfileEngine::diff(const Profile &a, const Profile &b)
{
    ProfileDelta delta;

    delta.dpiChanged = (a.dpi != b.dpi);

    delta.smartShiftChanged = (a.smartShiftEnabled   != b.smartShiftEnabled ||
                               a.smartShiftThreshold != b.smartShiftThreshold);

    delta.scrollChanged = (a.smoothScrolling  != b.smoothScrolling  ||
                           a.scrollDirection  != b.scrollDirection  ||
                           a.hiResScroll      != b.hiResScroll);

    delta.buttonsChanged  = (a.buttons  != b.buttons);
    delta.gesturesChanged = (a.gestures != b.gestures);

    return delta;
}

// ---------------------------------------------------------------------------
// ProfileEngine — instance methods
// ---------------------------------------------------------------------------

ProfileEngine::ProfileEngine(QObject *parent)
    : QObject(parent)
{
    m_debounceTimer.setSingleShot(true);
    m_debounceTimer.setInterval(200);

    connect(&m_debounceTimer, &QTimer::timeout, this, [this]() {
        doSwitchForApp(m_pendingAppClass);
    });
}

void ProfileEngine::setDeviceConfigDir(const QString &dir)
{
    m_configDir = dir;

    // Load app bindings if the file exists
    const QString bindingsFile = appBindingsPath();
    if (QFileInfo::exists(bindingsFile))
        m_appBindings = loadAppBindings(bindingsFile);
}

Profile ProfileEngine::activeProfile() const
{
    return m_activeProfile;
}

QStringList ProfileEngine::profileNames() const
{
    if (m_configDir.isEmpty())
        return {};

    QDir dir(m_configDir);
    const QStringList files = dir.entryList({"*.conf"}, QDir::Files);

    QStringList names;
    names.reserve(files.size());
    for (const QString &f : files)
        names << QFileInfo(f).baseName();
    return names;
}

void ProfileEngine::switchToProfile(const QString &name)
{
    const Profile prev = m_activeProfile;
    m_activeProfile = loadProfile(profilePath(name));
    const ProfileDelta delta = diff(prev, m_activeProfile);
    emit activeProfileChanged(m_activeProfile);
    emit profileDelta(delta, m_activeProfile);
}

void ProfileEngine::switchForApp(const QString &wmClass)
{
    m_pendingAppClass = wmClass;
    m_debounceTimer.start();
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

QString ProfileEngine::profilePath(const QString &name) const
{
    return m_configDir + "/" + name + ".conf";
}

QString ProfileEngine::appBindingsPath() const
{
    return m_configDir + "/app-bindings.conf";
}

void ProfileEngine::doSwitchForApp(const QString &wmClass)
{
    if (!m_appBindings.contains(wmClass))
        return;

    const QString profileName = m_appBindings.value(wmClass);
    switchToProfile(profileName);
}

} // namespace logitune
