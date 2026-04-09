#include <gtest/gtest.h>
#include "DeviceRegistry.h"

using namespace logitune;

TEST(DeviceRegistry, FindsMxMaster3sByPid) {
    DeviceRegistry reg;
    auto *dev = reg.findByPid(0xb034);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->deviceName(), "MX Master 3S");
}

TEST(DeviceRegistry, ReturnsNullForUnknownPid) {
    DeviceRegistry reg;
    EXPECT_EQ(reg.findByPid(0x0000), nullptr);
    EXPECT_EQ(reg.findByPid(0xFFFF), nullptr);
}

TEST(DeviceRegistry, ControlsHaveExpectedCids) {
    DeviceRegistry reg;
    auto *dev = reg.findByPid(0xb034);
    ASSERT_NE(dev, nullptr);
    auto controls = dev->controls();
    ASSERT_GE(controls.size(), 7);
    EXPECT_EQ(controls[0].controlId, 0x0050);
    EXPECT_EQ(controls[5].controlId, 0x00C3);
    EXPECT_EQ(controls[5].defaultActionType, "gesture-trigger");
    EXPECT_EQ(controls[6].defaultActionType, "smartshift-toggle");
}

TEST(DeviceRegistry, DefaultGesturesPresent) {
    DeviceRegistry reg;
    auto *dev = reg.findByPid(0xb034);
    ASSERT_NE(dev, nullptr);
    auto gestures = dev->defaultGestures();
    EXPECT_TRUE(gestures.contains("down"));
    EXPECT_EQ(gestures["down"].type, ButtonAction::Keystroke);
    EXPECT_EQ(gestures["down"].payload, "Super+D");
    EXPECT_EQ(gestures["up"].type, ButtonAction::Default);
}

TEST(DeviceRegistry, FeatureSupportCorrect) {
    DeviceRegistry reg;
    auto *dev = reg.findByPid(0xb034);
    ASSERT_NE(dev, nullptr);
    auto f = dev->features();
    EXPECT_TRUE(f.battery);
    EXPECT_TRUE(f.adjustableDpi);
    EXPECT_TRUE(f.smartShift);
    EXPECT_TRUE(f.reprogControls);
    EXPECT_FALSE(f.gestureV2);
}

TEST(DeviceRegistry, DpiRange) {
    DeviceRegistry reg;
    auto *dev = reg.findByPid(0xb034);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->minDpi(), 200);
    EXPECT_EQ(dev->maxDpi(), 8000);
    EXPECT_EQ(dev->dpiStep(), 50);
}

TEST(DeviceRegistry, HotspotsPresent) {
    DeviceRegistry reg;
    auto *dev = reg.findByPid(0xb034);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->buttonHotspots().size(), 6);
    EXPECT_EQ(dev->scrollHotspots().size(), 3);
}

TEST(DeviceRegistry, RenderComponentIsBasename) {
    // The descriptor should return a QML component basename (no path, no .qml),
    // which DeviceRender.qml resolves to the actual qrc URL.
    DeviceRegistry reg;
    auto *dev = reg.findByPid(0xb034);
    ASSERT_NE(dev, nullptr);
    QString rc = dev->renderComponent();
    EXPECT_FALSE(rc.isEmpty());
    EXPECT_FALSE(rc.contains('/'));       // not a path
    EXPECT_FALSE(rc.contains(".qml"));    // not a filename
    EXPECT_EQ(rc, "MxMaster3sRender");    // concrete value for MX Master 3S
}
