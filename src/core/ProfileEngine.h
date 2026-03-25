#pragma once
#include <QObject>
#include <QString>
#include <QMap>
#include <QTimer>
#include <QSettings>
#include <array>
#include <map>

namespace logitune {

struct ButtonAction {
    enum Type { Default, Keystroke, Media, DBus, GestureTrigger, AppLaunch };
    Type type = Default;
    QString payload;

    static ButtonAction parse(const QString &str);
    QString serialize() const;

    bool operator==(const ButtonAction &o) const {
        return type == o.type && payload == o.payload;
    }
    bool operator!=(const ButtonAction &o) const { return !(*this == o); }
};

struct Profile {
    int version = 1;
    QString name;
    QString icon;
    int dpi = 1000;
    bool smartShiftEnabled = true;
    int smartShiftThreshold = 128;
    bool smoothScrolling = false;
    QString scrollDirection = "standard";  // "standard" or "natural"
    bool hiResScroll = true;
    std::array<ButtonAction, 7> buttons;   // indexed 0-6
    std::map<QString, ButtonAction> gestures;  // "up","down","left","right","click"
};

struct ProfileDelta {
    bool dpiChanged = false;
    bool smartShiftChanged = false;
    bool scrollChanged = false;
    bool buttonsChanged = false;
    bool gesturesChanged = false;
};

class ProfileEngine : public QObject {
    Q_OBJECT
public:
    explicit ProfileEngine(QObject *parent = nullptr);

    // Static helpers (testable)
    static Profile loadProfile(const QString &path);
    static void saveProfile(const QString &path, const Profile &profile);
    static QMap<QString, QString> loadAppBindings(const QString &path);
    static void saveAppBindings(const QString &path, const QMap<QString, QString> &bindings);
    static ProfileDelta diff(const Profile &a, const Profile &b);

    // Instance methods
    void setDeviceConfigDir(const QString &dir);
    Profile activeProfile() const;
    QStringList profileNames() const;
    void switchToProfile(const QString &name);
    void switchForApp(const QString &wmClass);  // debounced

signals:
    void activeProfileChanged(const Profile &profile);
    void profileDelta(const ProfileDelta &delta, const Profile &newProfile);

private:
    QString m_configDir;
    Profile m_activeProfile;
    QMap<QString, QString> m_appBindings;
    QTimer m_debounceTimer;  // 200ms for app switching
    QString m_pendingAppClass;

    QString profilePath(const QString &name) const;
    QString appBindingsPath() const;
    void doSwitchForApp(const QString &wmClass);
};

} // namespace logitune
