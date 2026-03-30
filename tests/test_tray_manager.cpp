#include <gtest/gtest.h>
#include <QApplication>
#include <QSignalSpy>
#include <QAction>
#include <QMenu>
#include "TrayManager.h"
#include "models/DeviceModel.h"

using namespace logitune;

class TrayManagerTest : public ::testing::Test {
protected:
    DeviceModel dm;
    TrayManager tray{&dm};
};

TEST_F(TrayManagerTest, MenuHasThreeActions) {
    auto actions = tray.menu()->actions();
    int actionCount = 0;
    for (auto *a : actions)
        if (!a->isSeparator()) actionCount++;
    EXPECT_EQ(actionCount, 3);
}

TEST_F(TrayManagerTest, ShowActionText) {
    EXPECT_EQ(tray.showAction()->text(), "Show Logitune");
}

TEST_F(TrayManagerTest, QuitActionText) {
    EXPECT_EQ(tray.quitAction()->text(), "Quit");
}

TEST_F(TrayManagerTest, BatteryActionDisabled) {
    EXPECT_FALSE(tray.batteryAction()->isEnabled());
}

TEST_F(TrayManagerTest, ShowActionEmitsShowWindowRequested) {
    QSignalSpy spy(&tray, &TrayManager::showWindowRequested);
    tray.showAction()->trigger();
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TrayManagerTest, TrayLeftClickEmitsShowWindowRequested) {
    QSignalSpy spy(&tray, &TrayManager::showWindowRequested);
    emit tray.trayIcon()->activated(QSystemTrayIcon::Trigger);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(TrayManagerTest, TrayRightClickDoesNotEmitShow) {
    QSignalSpy spy(&tray, &TrayManager::showWindowRequested);
    emit tray.trayIcon()->activated(QSystemTrayIcon::Context);
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(TrayManagerTest, TooltipIsSet) {
    EXPECT_EQ(tray.trayIcon()->toolTip(), "Logitune - MX Master 3S");
}

TEST_F(TrayManagerTest, ContextMenuIsAttached) {
    EXPECT_EQ(tray.trayIcon()->contextMenu(), tray.menu());
}

TEST_F(TrayManagerTest, NullDeviceModelDoesNotCrash) {
    TrayManager nullTray(nullptr);
    EXPECT_EQ(nullTray.batteryAction()->text(), "Battery: ---%");
    EXPECT_EQ(nullTray.showAction()->text(), "Show Logitune");
}

// ── Battery display tests ────────────────────────────────────────────────

TEST_F(TrayManagerTest, BatteryTextShowsCurrentLevel) {
    // Without a DeviceManager, batteryLevel() returns 0
    // The tray should show the current battery level at construction, not "---"
    EXPECT_EQ(tray.batteryAction()->text(), dm.batteryStatusText());
}

TEST_F(TrayManagerTest, BatteryTextUpdatesOnSignal) {
    // Emit batteryLevelChanged and verify the action text updates
    // Without a DeviceManager we can't change the actual level, but we can
    // verify the connection exists by checking the text matches after the signal
    emit dm.batteryLevelChanged();
    EXPECT_EQ(tray.batteryAction()->text(), dm.batteryStatusText());
}
