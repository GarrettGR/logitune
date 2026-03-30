#pragma once
#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QDir>
#include <QTemporaryDir>
#include <memory>

#include "AppController.h"
#include "ProfileEngine.h"
#include "ButtonAction.h"
#include "helpers/TestFixtures.h"
#include "mocks/MockDesktop.h"
#include "mocks/MockInjector.h"
#include "mocks/MockDevice.h"

namespace logitune::test {

/// Integration-test fixture with friend access to AppController internals.
/// Provides mock desktop, mock injector, a temp profile directory, and helper
/// methods for creating profiles, simulating focus, pressing buttons, etc.
class AppControllerFixture : public ::testing::Test {
protected:
    void SetUp() override {
        ensureApp();
        ASSERT_TRUE(m_tmpDir.isValid());

        // Create mock dependencies (raw pointers — AppController does NOT own injected deps)
        m_desktop  = new MockDesktop();
        m_injector = new MockInjector();

        // Construct controller with DI
        m_ctrl = std::make_unique<AppController>(m_desktop, m_injector);
        m_ctrl->init();

        // Set up a temp profile directory with a default.conf
        m_profilesDir = m_tmpDir.path() + QStringLiteral("/profiles");
        QDir().mkpath(m_profilesDir);

        Profile defaultProfile;
        defaultProfile.name                = QStringLiteral("Default");
        defaultProfile.dpi                 = 1000;
        defaultProfile.smartShiftEnabled   = true;
        defaultProfile.smartShiftThreshold = 128;
        defaultProfile.smoothScrolling     = false;
        defaultProfile.scrollDirection     = QStringLiteral("standard");
        defaultProfile.hiResScroll         = true;
        defaultProfile.thumbWheelMode      = QStringLiteral("scroll");
        ProfileEngine::saveProfile(m_profilesDir + QStringLiteral("/default.conf"), defaultProfile);

        // Point ProfileEngine at the temp dir (loads default.conf into cache)
        m_ctrl->m_profileEngine.setDeviceConfigDir(m_profilesDir);

        // Set display + hardware profile to "default"
        m_ctrl->m_profileEngine.setDisplayProfile(QStringLiteral("default"));
        m_ctrl->m_profileEngine.setHardwareProfile(QStringLiteral("default"));

        // Create a MockDevice with MX Master 3S controls and install it
        m_device.setupMxControls();
        m_ctrl->m_currentDevice = &m_device;

        // Test environment: no real device, so defaultDirection=1 (neutral).
        // On real hardware, enumerateAndSetup reads this from ThumbWheel GetInfo.
        m_ctrl->m_deviceManager.m_thumbWheelDefaultDirection = 1;
    }

    void TearDown() override {
        m_ctrl.reset();
        delete m_desktop;
        m_desktop = nullptr;
        delete m_injector;
        m_injector = nullptr;
    }

    // -----------------------------------------------------------------------
    // Helper methods
    // -----------------------------------------------------------------------

    /// Creates an app profile in the cache and on disk, registers the
    /// wmClass -> profileName binding, and adds it to ProfileModel.
    void createAppProfile(const QString &wmClass,
                          const QString &profileName,
                          int dpi = 1000,
                          const QString &thumbMode = QStringLiteral("scroll"))
    {
        // Build a profile based on default, with overrides
        Profile p = m_ctrl->m_profileEngine.cachedProfile(QStringLiteral("default"));
        p.name           = profileName;
        p.dpi            = dpi;
        p.thumbWheelMode = thumbMode;

        // Save to disk
        ProfileEngine::saveProfile(m_profilesDir + "/" + profileName + ".conf", p);

        // Register binding via ProfileEngine (creates cache entry + app-bindings)
        m_ctrl->m_profileEngine.createProfileForApp(wmClass, profileName);

        // Add to ProfileModel (mirrors startup restore path)
        m_ctrl->m_profileModel.restoreProfile(wmClass, profileName);
    }

    void createAppProfile(const QString &wmClass,
                          const QString &profileName,
                          int dpi,
                          const QString &thumbMode,
                          bool thumbInvert,
                          bool smartShiftEnabled = true,
                          int smartShiftThreshold = 128,
                          const QString &scrollDirection = "standard",
                          bool hiResScroll = true)
    {
        Profile p = m_ctrl->m_profileEngine.cachedProfile(QStringLiteral("default"));
        p.name                = profileName;
        p.dpi                 = dpi;
        p.thumbWheelMode      = thumbMode;
        p.thumbWheelInvert    = thumbInvert;
        p.smartShiftEnabled   = smartShiftEnabled;
        p.smartShiftThreshold = smartShiftThreshold;
        p.scrollDirection     = scrollDirection;
        p.hiResScroll         = hiResScroll;

        // Create the cache entry and app binding first (copies from default)
        m_ctrl->m_profileEngine.createProfileForApp(wmClass, profileName);
        // Now overwrite the cache with our custom values
        Profile &cached = m_ctrl->m_profileEngine.cachedProfile(profileName);
        cached = p;
        // Save to disk with the correct values
        ProfileEngine::saveProfile(m_profilesDir + "/" + profileName + ".conf", p);
        m_ctrl->m_profileModel.restoreProfile(wmClass, profileName);
    }

    /// Modifies a button action in a cached profile and saves to disk.
    void setProfileButton(const QString &profileName, int buttonIdx, const ButtonAction &action) {
        Profile &p = m_ctrl->m_profileEngine.cachedProfile(profileName);
        if (buttonIdx >= 0 && buttonIdx < static_cast<int>(p.buttons.size()))
            p.buttons[static_cast<std::size_t>(buttonIdx)] = action;
        m_ctrl->m_profileEngine.saveProfileToDisk(profileName);
    }

    /// Sets a gesture keystroke on a cached profile and saves to disk.
    void setProfileGesture(const QString &profileName,
                           const QString &direction,
                           const QString &keystroke)
    {
        Profile &p = m_ctrl->m_profileEngine.cachedProfile(profileName);
        p.gestures[direction] = ButtonAction{ButtonAction::Keystroke, keystroke};
        m_ctrl->m_profileEngine.saveProfileToDisk(profileName);
    }

    /// Simulates a window focus change through MockDesktop and syncs the
    /// display profile to match the resulting hardware profile, so that
    /// DeviceModel display values reflect the active profile's settings.
    void focusApp(const QString &wmClass) {
        m_desktop->simulateFocus(wmClass, wmClass);
        // Sync display to hardware via the ProfileModel tab mechanism so that
        // both ProfileEngine and ProfileModel agree on the active display profile,
        // and DeviceModel getters (currentDPI, smartShiftEnabled, etc.) return
        // the focused profile's values.
        const QString hwProfile = m_ctrl->m_profileEngine.hardwareProfile();
        // Find the tab index in ProfileModel that corresponds to the hardware profile
        int hwIndex = 0; // default
        const int count = m_ctrl->m_profileModel.rowCount();
        for (int i = 0; i < count; ++i) {
            QModelIndex mi = m_ctrl->m_profileModel.index(i);
            if (m_ctrl->m_profileModel.data(mi, ProfileModel::WmClassRole).toString()
                    == hwProfile
                || m_ctrl->m_profileModel.data(mi, ProfileModel::NameRole).toString()
                    == hwProfile) {
                hwIndex = i;
                break;
            }
        }
        m_ctrl->m_profileModel.selectTab(hwIndex);
    }

    /// Simulates a diverted button press.
    void pressButton(uint16_t controlId) {
        m_ctrl->onDivertedButtonPressed(controlId, true);
    }

    /// Simulates a diverted button release.
    void releaseButton(uint16_t controlId) {
        m_ctrl->onDivertedButtonPressed(controlId, false);
    }

    /// Feeds raw gesture XY deltas into the controller.
    void gestureXY(int16_t dx, int16_t dy) {
        m_ctrl->onGestureRawXY(dx, dy);
    }

    /// Feeds a thumb wheel rotation delta into the controller.
    void thumbWheel(int delta) {
        m_ctrl->onThumbWheelRotation(delta);
    }

    // -----------------------------------------------------------------------
    // Accessors for private members (friend doesn't extend to TEST_F subclasses)
    // -----------------------------------------------------------------------

    ProfileEngine &profileEngine() { return m_ctrl->m_profileEngine; }
    ProfileModel  &profileModel()  { return m_ctrl->m_profileModel; }
    ButtonModel   &buttonModel()   { return m_ctrl->m_buttonModel; }
    DeviceModel   &deviceModel()   { return m_ctrl->m_deviceModel; }
    ActionModel   &actionModel()   { return m_ctrl->m_actionModel; }
    ActionExecutor &actionExecutor() { return m_ctrl->m_actionExecutor; }

    const IDevice *currentDevice() const { return m_ctrl->m_currentDevice; }

    // Gesture state
    int  gestureTotalDx() const { return m_ctrl->m_gestureTotalDx; }
    int  gestureTotalDy() const { return m_ctrl->m_gestureTotalDy; }
    bool gestureActive()  const { return m_ctrl->m_gestureActive; }

    // Thumb wheel state
    int thumbAccum() const { return m_ctrl->m_thumbAccum; }

    /// Directly set the DeviceManager's thumb wheel mode (bypasses hardware guards).
    void setThumbWheelMode(const QString &mode) { m_ctrl->m_deviceManager.m_thumbWheelMode = mode; }

    /// Directly set the DeviceManager's thumb wheel invert flag (bypasses hardware guards).
    void setThumbWheelInvert(bool invert) { m_ctrl->m_deviceManager.m_thumbWheelInvert = invert; }

    // -----------------------------------------------------------------------
    // Members
    // -----------------------------------------------------------------------
    MockDesktop  *m_desktop  = nullptr;
    MockInjector *m_injector = nullptr;
    MockDevice    m_device;
    std::unique_ptr<AppController> m_ctrl;
    QString       m_profilesDir;
    QTemporaryDir m_tmpDir;
};

} // namespace logitune::test
