#include "hidpp/features/Battery.h"

namespace logitune::hidpp::features {

BatteryStatus Battery::parseStatus(const Report &r)
{
    BatteryStatus status;
    status.level = static_cast<int>(r.params[0]);
    // UnifiedBattery (0x1004) getStatus/notification format:
    //   params[0] = state of charge (0-100%)
    //   params[1] = battery status (0=discharging, 8=charge complete, etc.)
    //   params[2] = external power status (0=none, 1=wired)
    status.charging = (r.params[2] != 0);
    return status;
}

} // namespace logitune::hidpp::features
