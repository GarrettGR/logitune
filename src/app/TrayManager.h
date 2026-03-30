#pragma once
#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>

namespace logitune {

class DeviceModel;

class TrayManager : public QObject
{
    Q_OBJECT
public:
    explicit TrayManager(DeviceModel *dm, QObject *parent = nullptr);

    QSystemTrayIcon *trayIcon() { return &m_trayIcon; }
    QMenu *menu() { return &m_menu; }
    QAction *showAction() { return m_showAction; }
    QAction *quitAction() { return m_quitAction; }
    QAction *batteryAction() { return m_batteryAction; }

    void show();

signals:
    void showWindowRequested();

private:
    QSystemTrayIcon m_trayIcon;
    QMenu m_menu;
    QAction *m_showAction = nullptr;
    QAction *m_batteryAction = nullptr;
    QAction *m_quitAction = nullptr;
};

} // namespace logitune
