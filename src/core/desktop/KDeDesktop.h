#pragma once
#include "interfaces/IDesktopIntegration.h"
#include <QDBusInterface>
#include <QTimer>

namespace logitune {

class KDeDesktop : public IDesktopIntegration {
    Q_OBJECT
public:
    explicit KDeDesktop(QObject *parent = nullptr);

    void start() override;
    bool available() const override;
    QString desktopName() const override;
    QStringList detectedCompositors() const override;
    void blockGlobalShortcuts(bool block) override;
    QVariantList runningApplications() const override;

public slots:
    // Called by KWin focus watcher script via D-Bus
    void focusChanged(const QString &resourceClass, const QString &title,
                      const QString &desktopFileName = QString());

private slots:
    void onActiveWindowChanged();
    void pollActiveWindow();

private:
    /// Resolve a resourceClass to a .desktop file completeBaseName by scanning
    /// installed .desktop files for matching StartupWMClass or name component.
    QString resolveDesktopFile(const QString &resourceClass) const;

    QDBusInterface *m_kwin = nullptr;
    QTimer *m_pollTimer = nullptr;
    QString m_lastWmClass;
    mutable QHash<QString, QString> m_resolveCache;  // resourceClass -> .desktop baseName
    bool m_available = false;
};

} // namespace logitune
