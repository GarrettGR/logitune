#include <gtest/gtest.h>
#include "hidpp/features/SmartShift.h"
#include "hidpp/features/HiResWheel.h"
#include "hidpp/features/ThumbWheel.h"

using namespace logitune::hidpp;
using namespace logitune::hidpp::features;

// ---------------------------------------------------------------------------
// SmartShift
// ---------------------------------------------------------------------------

TEST(SmartShift, ParseConfigEnabled)
{
    Report r;
    r.params[0]   = 0x01;  // enabled
    r.params[1]   = 30;    // threshold
    r.paramLength = 4;
    auto cfg = SmartShift::parseConfig(r);
    EXPECT_TRUE(cfg.enabled);
    EXPECT_EQ(cfg.threshold, 30);
}

TEST(SmartShift, ParseConfigDisabled)
{
    Report r;
    r.params[0]   = 0x00;  // disabled
    r.params[1]   = 100;
    r.paramLength = 4;
    auto cfg = SmartShift::parseConfig(r);
    EXPECT_FALSE(cfg.enabled);
    EXPECT_EQ(cfg.threshold, 100);
}

TEST(SmartShift, BuildSetConfig)
{
    auto params = SmartShift::buildSetConfig(true, 50);
    ASSERT_EQ(params.size(), 3u);
    EXPECT_EQ(params[0], 0x02);
    EXPECT_EQ(params[1], 0x01);
    EXPECT_EQ(params[2], 50);
}

TEST(SmartShift, BuildSetConfigDisabled)
{
    auto params = SmartShift::buildSetConfig(false, 10);
    ASSERT_EQ(params.size(), 3u);
    EXPECT_EQ(params[0], 0x02);
    EXPECT_EQ(params[1], 0x00);
    EXPECT_EQ(params[2], 10);
}

TEST(SmartShift, ConstantValues)
{
    EXPECT_EQ(SmartShift::kFnGetConfig, 0x00);
    EXPECT_EQ(SmartShift::kFnSetConfig, 0x01);
}

// ---------------------------------------------------------------------------
// HiResWheel
// ---------------------------------------------------------------------------

TEST(HiResWheel, ParseConfigBits)
{
    Report r;
    // bit 1 = hiRes, bit 2 = invert (per Solaar 0x2121)
    r.params[0] = 0x06;  // bits 1+2 set
    auto cfg = HiResWheel::parseWheelMode(r);
    EXPECT_TRUE(cfg.hiRes);
    EXPECT_TRUE(cfg.invert);
}

TEST(HiResWheel, ParseConfigHiResOnly)
{
    Report r;
    r.params[0] = 0x02;  // bit 1 only
    auto cfg = HiResWheel::parseWheelMode(r);
    EXPECT_TRUE(cfg.hiRes);
    EXPECT_FALSE(cfg.invert);
}

TEST(HiResWheel, ParseConfigNoBitsSet)
{
    Report r;
    r.params[0] = 0x00;
    auto cfg = HiResWheel::parseWheelMode(r);
    EXPECT_FALSE(cfg.hiRes);
    EXPECT_FALSE(cfg.invert);
}

TEST(HiResWheel, ParseRatchetSwitch)
{
    Report r;
    r.params[0] = 0x01;
    EXPECT_TRUE(HiResWheel::parseRatchetSwitch(r));
    r.params[0] = 0x00;
    EXPECT_FALSE(HiResWheel::parseRatchetSwitch(r));
}

TEST(HiResWheel, BuildSetBothSet)
{
    // hiRes=bit1, invert=bit2 → 0x06
    auto params = HiResWheel::buildSetWheelMode(0x00, true, true);
    ASSERT_EQ(params.size(), 1u);
    EXPECT_EQ(params[0], 0x06);
}

TEST(HiResWheel, BuildSetHiResOnly)
{
    auto params = HiResWheel::buildSetWheelMode(0x00, true, false);
    ASSERT_EQ(params.size(), 1u);
    EXPECT_EQ(params[0], 0x02);
}

TEST(HiResWheel, BuildSetInvertOnly)
{
    auto params = HiResWheel::buildSetWheelMode(0x00, false, true);
    ASSERT_EQ(params.size(), 1u);
    EXPECT_EQ(params[0], 0x04);
}

TEST(HiResWheel, BuildPreservesDiversionBit)
{
    // currentMode has diversion bit set (0x01)
    auto params = HiResWheel::buildSetWheelMode(0x01, true, false);
    ASSERT_EQ(params.size(), 1u);
    EXPECT_EQ(params[0], 0x03); // diversion(0x01) + hiRes(0x02)
}

TEST(HiResWheel, ConstantValues)
{
    EXPECT_EQ(HiResWheel::kFnGetCapabilities,  0x00);
    EXPECT_EQ(HiResWheel::kFnGetWheelMode,     0x01);
    EXPECT_EQ(HiResWheel::kFnSetWheelMode,     0x02);
    EXPECT_EQ(HiResWheel::kFnGetRatchetSwitch, 0x03);
}

// ---------------------------------------------------------------------------
// ThumbWheel
// ---------------------------------------------------------------------------

TEST(ThumbWheel, ParseConfigInverted)
{
    Report r;
    r.params[0]   = 0x01;  // invert bit set
    r.params[1]   = 75;    // resolution
    r.paramLength = 4;
    auto cfg = ThumbWheel::parseConfig(r);
    EXPECT_TRUE(cfg.invert);
    EXPECT_EQ(cfg.resolution, 75);
}

TEST(ThumbWheel, ParseConfigNotInverted)
{
    Report r;
    r.params[0]   = 0x00;
    r.params[1]   = 50;
    r.paramLength = 4;
    auto cfg = ThumbWheel::parseConfig(r);
    EXPECT_FALSE(cfg.invert);
    EXPECT_EQ(cfg.resolution, 50);
}

TEST(ThumbWheel, BuildSetConfigInvert)
{
    auto params = ThumbWheel::buildSetConfig(true);
    ASSERT_EQ(params.size(), 1u);
    EXPECT_EQ(params[0], 0x01);
}

TEST(ThumbWheel, BuildSetConfigNoInvert)
{
    auto params = ThumbWheel::buildSetConfig(false);
    ASSERT_EQ(params.size(), 1u);
    EXPECT_EQ(params[0], 0x00);
}

TEST(ThumbWheel, ConstantValues)
{
    EXPECT_EQ(ThumbWheel::kFnGetConfig, 0x00);
    EXPECT_EQ(ThumbWheel::kFnSetConfig, 0x01);
}
