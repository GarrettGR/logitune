#include <gtest/gtest.h>
#include "hidpp/capabilities/Capabilities.h"
#include "hidpp/FeatureDispatcher.h"

using namespace logitune::hidpp;
using namespace logitune::hidpp::capabilities;

namespace {

// Minimal test variant struct matching the shape real variants use.
struct TestVariant {
    FeatureId feature;
    int       tag;   // differentiator for assertions
};

constexpr TestVariant kTestVariants[] = {
    { FeatureId::BatteryUnified, 1 },
    { FeatureId::BatteryStatus,  2 },
};

} // namespace

TEST(ResolveCapability, ReturnsFirstMatch) {
    FeatureDispatcher fd;
    fd.setFeatureTable({
        {FeatureId::BatteryUnified, 0x02},
    });
    auto v = resolveCapability(&fd, kTestVariants);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->feature, FeatureId::BatteryUnified);
    EXPECT_EQ(v->tag, 1);
}

TEST(ResolveCapability, FallsBackToSecondMatch) {
    FeatureDispatcher fd;
    fd.setFeatureTable({
        {FeatureId::BatteryStatus, 0x02},
    });
    auto v = resolveCapability(&fd, kTestVariants);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->feature, FeatureId::BatteryStatus);
    EXPECT_EQ(v->tag, 2);
}

TEST(ResolveCapability, PrefersFirstWhenBothPresent) {
    FeatureDispatcher fd;
    fd.setFeatureTable({
        {FeatureId::BatteryUnified, 0x02},
        {FeatureId::BatteryStatus,  0x03},
    });
    auto v = resolveCapability(&fd, kTestVariants);
    ASSERT_TRUE(v.has_value());
    EXPECT_EQ(v->feature, FeatureId::BatteryUnified);
    EXPECT_EQ(v->tag, 1);
}

TEST(ResolveCapability, ReturnsNulloptWhenNoneMatch) {
    FeatureDispatcher fd;
    fd.setFeatureTable({
        {FeatureId::GestureV2, 0x02},
    });
    auto v = resolveCapability(&fd, kTestVariants);
    EXPECT_FALSE(v.has_value());
}

TEST(ResolveCapability, ReturnsNulloptWhenDispatcherEmpty) {
    FeatureDispatcher fd;
    auto v = resolveCapability(&fd, kTestVariants);
    EXPECT_FALSE(v.has_value());
}
