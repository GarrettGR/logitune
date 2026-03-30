#include "TrayManager.h"
#include "models/DeviceModel.h"
#include <QIcon>

namespace logitune {

TrayManager::TrayManager(DeviceModel *dm, QObject *parent)
    : QObject(parent)
{
    m_trayIcon.setIcon(QIcon::fromTheme("input-mouse"));
    m_trayIcon.setToolTip("Logitune - MX Master 3S");

    m_showAction = m_menu.addAction("Show Logitune");
    m_menu.addSeparator();
    m_batteryAction = m_menu.addAction("Battery: ---%");
    m_batteryAction->setEnabled(false);
    m_menu.addSeparator();
    m_quitAction = m_menu.addAction("Quit");
    m_trayIcon.setContextMenu(&m_menu);

    connect(m_showAction, &QAction::triggered, this, &TrayManager::showWindowRequested);
    connect(&m_trayIcon, &QSystemTrayIcon::activated, this,
        [this](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger)
                emit showWindowRequested();
        });

    if (dm) {
        m_batteryAction->setText(dm->batteryStatusText());
        connect(dm, &DeviceModel::batteryLevelChanged, this,
            [this, dm]() { m_batteryAction->setText(dm->batteryStatusText()); });
    }
}

void TrayManager::show()
{
    m_trayIcon.show();
}

} // namespace logitune
