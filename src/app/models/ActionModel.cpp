#include "ActionModel.h"

namespace logitune {

ActionModel::ActionModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_actions = {
        { "Advanced click",       "Simulate advanced click patterns",          "advanced-click"  },
        { "App expose",           "Show all windows for current app",           "app-expose"      },
        { "Back",                 "Navigate backward in browser/file manager",  "default"         },
        { "Brightness down",      "Decrease display brightness",                "brightness-down" },
        { "Brightness up",        "Increase display brightness",                "brightness-up"   },
        { "Calculator",           "Open the system calculator",                 "calculator"      },
        { "Change pointer speed", "Adjust pointer sensitivity on the fly",      "pointer-speed"   },
        { "Close window",         "Close the active window",                    "close-window"    },
        { "Copy",                 "Copy selected content to clipboard",         "keystroke"       },
        { "Cut",                  "Cut selected content to clipboard",          "keystroke"       },
        { "Do nothing",           "Button is disabled",                         "none"            },
        { "Forward",              "Navigate forward in browser/file manager",   "default"         },
        { "Gestures",             "Trigger gesture recognition",                "gesture-trigger" },
        { "Keyboard shortcut",    "Send a custom key combination",              "keystroke"       },
        { "Media controls",       "Control media playback",                     "media"           },
        { "Middle click",         "Emulate middle mouse button click",          "default"         },
        { "Mute",                 "Toggle audio mute",                          "mute"            },
        { "Paste",                "Paste clipboard content",                    "keystroke"       },
        { "Play/Pause",           "Play or pause media",                        "media"           },
        { "Redo",                 "Redo last undone action",                    "keystroke"       },
        { "Screenshot",           "Capture a screenshot",                       "screenshot"      },
        { "Show desktop",         "Minimize all windows to show desktop",       "show-desktop"    },
        { "Undo",                 "Undo the last action",                       "keystroke"       },
        { "Volume down",          "Decrease system volume",                     "volume-down"     },
        { "Volume up",            "Increase system volume",                     "volume-up"       },
    };
}

int ActionModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_actions.size();
}

QVariant ActionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_actions.size())
        return {};

    const ActionEntry &entry = m_actions[index.row()];
    switch (role) {
    case NameRole:        return entry.name;
    case DescriptionRole: return entry.description;
    case ActionTypeRole:  return entry.actionType;
    default:              return {};
    }
}

QHash<int, QByteArray> ActionModel::roleNames() const
{
    return {
        { NameRole,        "name"        },
        { DescriptionRole, "description" },
        { ActionTypeRole,  "actionType"  },
    };
}

int ActionModel::selectedIndex() const
{
    return m_selectedIndex;
}

void ActionModel::setSelectedIndex(int idx)
{
    if (m_selectedIndex == idx)
        return;
    m_selectedIndex = idx;
    emit selectedIndexChanged();
}

int ActionModel::indexForName(const QString &name) const
{
    for (int i = 0; i < m_actions.size(); ++i) {
        if (m_actions[i].name == name)
            return i;
    }
    return -1;
}

} // namespace logitune
