#pragma once
#include "hidpp/HidppTypes.h"
#include "ButtonAction.h"
#include <QString>
#include <QList>
#include <QMap>
#include <vector>

namespace logitune {

struct ControlDescriptor {
    uint16_t controlId;
    int buttonIndex;
    QString defaultName;
    QString defaultActionType;
    bool configurable;
};

struct HotspotDescriptor {
    int buttonIndex;
    double xPct;
    double yPct;
    QString side;
    double labelOffsetYPct;
};

struct FeatureSupport {
    bool battery = false;
    bool adjustableDpi = false;
    bool smartShift = false;
    bool hiResWheel = false;
    bool thumbWheel = false;
    bool reprogControls = false;
    bool gestureV2 = false;
};

class IDevice {
public:
    virtual ~IDevice() = default;

    virtual QString deviceName() const = 0;
    virtual std::vector<uint16_t> productIds() const = 0;
    virtual bool matchesPid(uint16_t pid) const = 0;
    virtual QList<ControlDescriptor> controls() const = 0;
    virtual QList<HotspotDescriptor> buttonHotspots() const = 0;
    virtual QList<HotspotDescriptor> scrollHotspots() const = 0;
    virtual FeatureSupport features() const = 0;
    virtual QString frontImagePath() const = 0;
    virtual QString sideImagePath() const = 0;
    virtual QString backImagePath() const = 0;
    virtual QMap<QString, ButtonAction> defaultGestures() const = 0;
    virtual int minDpi() const = 0;
    virtual int maxDpi() const = 0;
    virtual int dpiStep() const = 0;
    virtual int easySwitchSlots() const = 0;

    /// Returns the basename of this device's QML render component
    /// (e.g. "MxMaster3sRender"). The file must live under
    /// src/app/qml/devices/<basename>.qml and be registered in
    /// src/app/CMakeLists.txt. The component must accept `view` (string)
    /// and `showHotspots` (bool) properties and emit `buttonClicked(int)`
    /// if it has clickable button hotspots.
    virtual QString renderComponent() const = 0;
};

} // namespace logitune
