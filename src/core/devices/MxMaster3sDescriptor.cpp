#include "MxMaster3sDescriptor.h"

namespace logitune {

QString MxMaster3sDescriptor::deviceName() const
{
    return QStringLiteral("MX Master 3S");
}

std::vector<uint16_t> MxMaster3sDescriptor::productIds() const
{
    return { 0xb034 };  // Bolt receiver-reported PID; BT PID discovered via device name
}

bool MxMaster3sDescriptor::matchesPid(uint16_t pid) const
{
    for (auto id : productIds()) {
        if (id == pid)
            return true;
    }
    return false;
}

QList<ControlDescriptor> MxMaster3sDescriptor::controls() const
{
    return {
        { 0x0050, 0, QStringLiteral("Left click"),        QStringLiteral("default"),          false },
        { 0x0051, 1, QStringLiteral("Right click"),       QStringLiteral("default"),          false },
        { 0x0052, 2, QStringLiteral("Middle click"),      QStringLiteral("default"),          true  },
        { 0x0053, 3, QStringLiteral("Back"),              QStringLiteral("default"),          true  },
        { 0x0056, 4, QStringLiteral("Forward"),           QStringLiteral("default"),          true  },
        { 0x00C3, 5, QStringLiteral("Gesture button"),    QStringLiteral("gesture-trigger"),  true  },
        { 0x00C4, 6, QStringLiteral("Shift wheel mode"),  QStringLiteral("smartshift-toggle"),true  },
        { 0x0000, 7, QStringLiteral("Thumb wheel"),       QStringLiteral("default"),          true  },
    };
}

QList<HotspotDescriptor> MxMaster3sDescriptor::buttonHotspots() const
{
    return {
        { 2, 0.71, 0.15,  QStringLiteral("right"), 0.0  },
        { 6, 0.81, 0.34,  QStringLiteral("right"), 0.0  },
        { 7, 0.55, 0.515, QStringLiteral("right"), 0.0  },
        { 4, 0.35, 0.43,  QStringLiteral("left"),  0.0  },
        { 3, 0.45, 0.60,  QStringLiteral("left"),  0.20 },
        { 5, 0.08, 0.58,  QStringLiteral("left"),  0.0  },
    };
}

QList<HotspotDescriptor> MxMaster3sDescriptor::scrollHotspots() const
{
    return {
        { -1, 0.73, 0.16, QStringLiteral("right"), 0.0 },
        { -2, 0.55, 0.51, QStringLiteral("left"),  0.0 },
        { -3, 0.83, 0.54, QStringLiteral("right"), 0.0 },
    };
}

FeatureSupport MxMaster3sDescriptor::features() const
{
    FeatureSupport f;
    f.battery        = true;
    f.adjustableDpi  = true;
    f.smartShift     = true;
    f.hiResWheel     = true;
    f.thumbWheel     = true;
    f.reprogControls = true;
    f.gestureV2      = false;
    return f;
}

QString MxMaster3sDescriptor::frontImagePath() const
{
    return QStringLiteral("qrc:/Logitune/qml/assets/mx-master-3s.png");
}

QString MxMaster3sDescriptor::sideImagePath() const
{
    return QStringLiteral("qrc:/Logitune/qml/assets/mx-master-3s-side.png");
}

QString MxMaster3sDescriptor::backImagePath() const
{
    return QStringLiteral("qrc:/Logitune/qml/assets/mx-master-3s-back.png");
}

QMap<QString, ButtonAction> MxMaster3sDescriptor::defaultGestures() const
{
    QMap<QString, ButtonAction> g;
    g[QStringLiteral("up")]    = { ButtonAction::Default,   {} };
    g[QStringLiteral("down")]  = { ButtonAction::Keystroke, QStringLiteral("Super+D") };
    g[QStringLiteral("left")]  = { ButtonAction::Keystroke, QStringLiteral("Ctrl+Super+Left") };
    g[QStringLiteral("right")] = { ButtonAction::Keystroke, QStringLiteral("Ctrl+Super+Right") };
    g[QStringLiteral("click")] = { ButtonAction::Keystroke, QStringLiteral("Super+W") };
    return g;
}

int MxMaster3sDescriptor::minDpi() const  { return 200; }
int MxMaster3sDescriptor::maxDpi() const  { return 8000; }
int MxMaster3sDescriptor::dpiStep() const { return 50; }

int MxMaster3sDescriptor::easySwitchSlots() const { return 3; }

QString MxMaster3sDescriptor::renderComponent() const
{
    // Basename of the QML file under qml/devices/ (no path, no extension).
    // DeviceRender.qml resolves it to the correct qrc URL for the current Qt version.
    return QStringLiteral("MxMaster3sRender");
}

} // namespace logitune
