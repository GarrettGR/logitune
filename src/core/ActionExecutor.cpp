#include "ActionExecutor.h"
#include "input/UinputInjector.h"

namespace logitune {

// ---------------------------------------------------------------------------
// GestureDetector
// ---------------------------------------------------------------------------

void GestureDetector::reset()
{
    m_totalDx = 0;
    m_totalDy = 0;
}

void GestureDetector::addDelta(int dx, int dy)
{
    m_totalDx += dx;
    m_totalDy += dy;
}

GestureDirection GestureDetector::resolve() const
{
    const int absDx = m_totalDx < 0 ? -m_totalDx : m_totalDx;
    const int absDy = m_totalDy < 0 ? -m_totalDy : m_totalDy;

    if (absDx <= kThreshold && absDy <= kThreshold)
        return GestureDirection::Click;

    // Dominant axis wins
    if (absDx >= absDy) {
        return m_totalDx > 0 ? GestureDirection::Right : GestureDirection::Left;
    } else {
        return m_totalDy > 0 ? GestureDirection::Down : GestureDirection::Up;
    }
}

// ---------------------------------------------------------------------------
// ActionExecutor — construction
// ---------------------------------------------------------------------------

ActionExecutor::ActionExecutor(IInputInjector *injector, QObject *parent)
    : QObject(parent)
    , m_injector(injector)
{
}

void ActionExecutor::setInjector(IInputInjector *injector)
{
    m_injector = injector;
}

// ---------------------------------------------------------------------------
// Action dispatch
// ---------------------------------------------------------------------------

void ActionExecutor::executeAction(const ButtonAction &action)
{
    switch (action.type) {
    case ButtonAction::Keystroke:
        injectKeystroke(action.payload);
        break;
    case ButtonAction::DBus:
        executeDBusCall(action.payload);
        break;
    case ButtonAction::AppLaunch:
        launchApp(action.payload);
        break;
    case ButtonAction::Default:
    case ButtonAction::GestureTrigger:
    case ButtonAction::SmartShiftToggle:
    case ButtonAction::Media:
    default:
        break;
    }
}

void ActionExecutor::injectKeystroke(const QString &combo)
{
    m_injector->injectKeystroke(combo);
}

void ActionExecutor::injectCtrlScroll(int direction)
{
    m_injector->injectCtrlScroll(direction);
}

void ActionExecutor::executeDBusCall(const QString &spec)
{
    m_injector->sendDBusCall(spec);
}

void ActionExecutor::launchApp(const QString &command)
{
    m_injector->launchApp(command);
}

GestureDetector &ActionExecutor::gestureDetector()
{
    return m_gestureDetector;
}

// ---------------------------------------------------------------------------
// Static helpers
// ---------------------------------------------------------------------------

std::vector<int> ActionExecutor::parseKeystroke(const QString &combo)
{
    return UinputInjector::parseKeystroke(combo);
}

DBusCall ActionExecutor::parseDBusAction(const QString &spec)
{
    const QStringList parts = spec.split(QLatin1Char(','));
    if (parts.size() != 4)
        return {};   // method will be empty, caller treats as invalid

    return DBusCall{
        parts[0].trimmed(),
        parts[1].trimmed(),
        parts[2].trimmed(),
        parts[3].trimmed(),
    };
}

QString ActionExecutor::gestureDirectionName(GestureDirection dir)
{
    switch (dir) {
    case GestureDirection::None:  return QStringLiteral("None");
    case GestureDirection::Up:    return QStringLiteral("Up");
    case GestureDirection::Down:  return QStringLiteral("Down");
    case GestureDirection::Left:  return QStringLiteral("Left");
    case GestureDirection::Right: return QStringLiteral("Right");
    case GestureDirection::Click: return QStringLiteral("Click");
    }
    return QStringLiteral("None");
}

} // namespace logitune
