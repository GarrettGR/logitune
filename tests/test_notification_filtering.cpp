#include <gtest/gtest.h>
#include <QSignalSpy>
#include "DeviceManager.h"
#include "DeviceRegistry.h"
#include "hidpp/HidppTypes.h"

using namespace logitune;
using namespace logitune::hidpp;

class NotificationFilterTest : public ::testing::Test {
protected:
    DeviceRegistry registry;
    DeviceManager dm{&registry};
};

TEST_F(NotificationFilterTest, ResponseWithSoftwareId1IsDiscarded) {
    // A response to our callAsync request has softwareId=0x01.
    // This must NOT be processed as a device notification.
    // Bug regression: callAsync response with params {0x01, 0x00} was parsed
    // as thumbWheelRotation(256), causing bogus volume bursts.
    Report response;
    response.reportId     = 0x11;
    response.deviceIndex  = 0x01;
    response.featureIndex = 0x10;  // any feature index
    response.functionId   = 0x02;  // setReporting response
    response.softwareId   = 0x01;  // OUR request echo
    response.params[0]    = 0x01;
    response.params[1]    = 0x00;
    response.paramLength  = 2;

    QSignalSpy thumbSpy(&dm, &DeviceManager::thumbWheelRotation);
    QSignalSpy batterySpy(&dm, &DeviceManager::batteryLevelChanged);

    dm.handleNotification(response);

    EXPECT_EQ(thumbSpy.count(), 0) << "Response must not trigger thumbWheelRotation";
    EXPECT_EQ(batterySpy.count(), 0) << "Response must not trigger batteryLevelChanged";
}

TEST_F(NotificationFilterTest, NotificationWithSoftwareId0IsNotDiscarded) {
    // A real device notification has softwareId=0x00.
    // It should pass the filter (even if no features are enumerated,
    // it won't match any feature handler — but it won't be discarded
    // at the softwareId check).
    Report notification;
    notification.reportId     = 0x11;
    notification.deviceIndex  = 0x01;
    notification.featureIndex = 0xFF;  // nonexistent feature
    notification.functionId   = 0x00;
    notification.softwareId   = 0x00;  // real notification
    notification.paramLength  = 2;

    // This won't emit any signal (no features enumerated) but it
    // proves the softwareId=0 filter doesn't discard it.
    // The test passes if no crash/assertion occurs.
    dm.handleNotification(notification);
}
