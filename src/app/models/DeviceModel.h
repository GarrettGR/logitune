#pragma once
#include "DeviceManager.h"
#include <QMap>
#include <QPair>
#include <QObject>
#include <qqmlintegration.h>

namespace logitune {

class DeviceModel : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool deviceConnected READ deviceConnected NOTIFY deviceConnectedChanged)
    Q_PROPERTY(QString deviceName READ deviceName NOTIFY deviceNameChanged)
    Q_PROPERTY(int batteryLevel READ batteryLevel NOTIFY batteryLevelChanged)
    Q_PROPERTY(bool batteryCharging READ batteryCharging NOTIFY batteryChargingChanged)
    Q_PROPERTY(QString connectionType READ connectionType NOTIFY connectionTypeChanged)
    Q_PROPERTY(int currentDPI READ currentDPI NOTIFY currentDPIChanged)
    Q_PROPERTY(int minDPI READ minDPI CONSTANT)
    Q_PROPERTY(int maxDPI READ maxDPI CONSTANT)
    Q_PROPERTY(int dpiStep READ dpiStep CONSTANT)
    Q_PROPERTY(bool smartShiftEnabled READ smartShiftEnabled NOTIFY smartShiftEnabledChanged)
    Q_PROPERTY(int smartShiftThreshold READ smartShiftThreshold NOTIFY smartShiftThresholdChanged)
    Q_PROPERTY(bool scrollHiRes READ scrollHiRes NOTIFY scrollConfigChanged)
    Q_PROPERTY(bool scrollInvert READ scrollInvert NOTIFY scrollConfigChanged)
    Q_PROPERTY(QString activeProfileName READ activeProfileName NOTIFY activeProfileNameChanged)

public:
    explicit DeviceModel(QObject *parent = nullptr);

    void setDeviceManager(DeviceManager *dm);

    bool deviceConnected() const;
    QString deviceName() const;
    int batteryLevel() const;
    bool batteryCharging() const;
    QString connectionType() const;
    int currentDPI() const;
    int minDPI() const;
    int maxDPI() const;
    int dpiStep() const;
    bool smartShiftEnabled() const;
    int smartShiftThreshold() const;
    QString activeProfileName() const;

    Q_INVOKABLE void setDPI(int value);
    Q_INVOKABLE void setSmartShift(bool enabled, int threshold);
    Q_INVOKABLE void setScrollConfig(bool hiRes, bool invert);
    Q_INVOKABLE void setThumbWheelMode(const QString &mode);
    Q_INVOKABLE void resetAllProfiles();
    Q_INVOKABLE void setGestureAction(const QString &direction, const QString &actionName, const QString &keystroke);
    Q_INVOKABLE QString gestureActionName(const QString &direction) const;
    Q_INVOKABLE QString gestureKeystroke(const QString &direction) const;
    Q_PROPERTY(QString thumbWheelMode READ thumbWheelMode NOTIFY thumbWheelModeChanged)
    bool scrollHiRes() const;
    bool scrollInvert() const;
    QString thumbWheelMode() const;

    // Called from main integration to sync profile state into the model
    void setCurrentDPI(int dpi);
    void setSmartShiftState(bool enabled, int threshold);
    void setActiveProfileName(const QString &name);

signals:
    void deviceConnectedChanged();
    void deviceNameChanged();
    void batteryLevelChanged();
    void batteryChargingChanged();
    void connectionTypeChanged();
    void currentDPIChanged();
    void smartShiftEnabledChanged();
    void smartShiftThresholdChanged();
    void scrollConfigChanged();
    void thumbWheelModeChanged();
    void activeProfileNameChanged();
    void gestureChanged();

private:
    DeviceManager *m_dm = nullptr;
    QMap<QString, QPair<QString, QString>> m_gestures; // direction → (actionName, keystroke)
    int m_currentDPI = 1000;
    bool m_smartShiftEnabled = true;
    int m_smartShiftThreshold = 128;
    QString m_activeProfileName;
};

} // namespace logitune
