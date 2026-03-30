#include <gtest/gtest.h>
#include "helpers/AppControllerFixture.h"

using namespace logitune;
using namespace logitune::test;

// =============================================================================
// Volume mode — direction
// =============================================================================

TEST_F(AppControllerFixture, VolumeClockwiseIsVolumeUp) {
    setThumbWheelMode("volume");
    thumbWheel(20);
    EXPECT_TRUE(m_injector->hasCalled("injectKeystroke"));
    EXPECT_EQ(m_injector->lastArg("injectKeystroke"), "VolumeUp");
}

TEST_F(AppControllerFixture, VolumeAntiClockwiseIsVolumeDown) {
    setThumbWheelMode("volume");
    thumbWheel(-20);
    EXPECT_TRUE(m_injector->hasCalled("injectKeystroke"));
    EXPECT_EQ(m_injector->lastArg("injectKeystroke"), "VolumeDown");
}

// =============================================================================
// Zoom mode — direction
// =============================================================================

TEST_F(AppControllerFixture, ZoomClockwiseIsZoomIn) {
    setThumbWheelMode("zoom");
    thumbWheel(20);
    EXPECT_TRUE(m_injector->hasCalled("injectCtrlScroll"));
    EXPECT_EQ(m_injector->lastArg("injectCtrlScroll"), "1");
}

TEST_F(AppControllerFixture, ZoomAntiClockwiseIsZoomOut) {
    setThumbWheelMode("zoom");
    thumbWheel(-20);
    EXPECT_TRUE(m_injector->hasCalled("injectCtrlScroll"));
    EXPECT_EQ(m_injector->lastArg("injectCtrlScroll"), "-1");
}

// =============================================================================
// Scroll mode — horizontal scroll, no volume/zoom
// =============================================================================

TEST_F(AppControllerFixture, ScrollModeInjectsHorizontalScroll) {
    setThumbWheelMode("scroll");
    thumbWheel(20);
    EXPECT_TRUE(m_injector->hasCalled("injectHorizontalScroll"));
    EXPECT_FALSE(m_injector->hasCalled("injectKeystroke"));
    EXPECT_FALSE(m_injector->hasCalled("injectCtrlScroll"));
}

TEST_F(AppControllerFixture, ScrollModeClockwiseIsScrollRight) {
    setThumbWheelMode("scroll");
    thumbWheel(20);
    EXPECT_EQ(m_injector->lastArg("injectHorizontalScroll"), "1");
}

TEST_F(AppControllerFixture, ScrollModeAntiClockwiseIsScrollLeft) {
    setThumbWheelMode("scroll");
    thumbWheel(-20);
    EXPECT_EQ(m_injector->lastArg("injectHorizontalScroll"), "-1");
}

// =============================================================================
// Threshold — boundary conditions
// =============================================================================

TEST_F(AppControllerFixture, ThumbWheelExactThresholdDispatches) {
    setThumbWheelMode("volume");
    thumbWheel(15);
    EXPECT_TRUE(m_injector->hasCalled("injectKeystroke"));
}

TEST_F(AppControllerFixture, ThumbWheelBelowThresholdDoesNotDispatch) {
    setThumbWheelMode("volume");
    thumbWheel(14);
    EXPECT_FALSE(m_injector->hasCalled("injectKeystroke"));
}

TEST_F(AppControllerFixture, ThumbWheelNegativeThresholdDispatches) {
    setThumbWheelMode("volume");
    thumbWheel(-15);
    EXPECT_TRUE(m_injector->hasCalled("injectKeystroke"));
    EXPECT_EQ(m_injector->lastArg("injectKeystroke"), "VolumeDown");
}

TEST_F(AppControllerFixture, ThumbWheelAccumulatesAcrossMultipleDeltas) {
    setThumbWheelMode("volume");
    thumbWheel(8);
    EXPECT_FALSE(m_injector->hasCalled("injectKeystroke"));
    thumbWheel(8);  // total = 16
    EXPECT_TRUE(m_injector->hasCalled("injectKeystroke"));
}

// =============================================================================
// Profile switch changes thumb wheel mode
// =============================================================================

TEST_F(AppControllerFixture, ProfileSwitchChangesThumbWheelFromVolumeToScroll) {
    createAppProfile("google-chrome", "Chrome", 1000, "volume", false);
    createAppProfile("kwrite", "KWrite", 1000, "scroll", false);

    focusApp("google-chrome");
    thumbWheel(20);
    EXPECT_TRUE(m_injector->hasCalled("injectKeystroke"));
    EXPECT_EQ(m_injector->lastArg("injectKeystroke"), "VolumeUp");

    m_injector->clear();

    focusApp("kwrite");
    thumbWheel(20);
    EXPECT_TRUE(m_injector->hasCalled("injectHorizontalScroll"));
    EXPECT_FALSE(m_injector->hasCalled("injectKeystroke"));
}
