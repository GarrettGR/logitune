import QtQuick
import QtQuick.Controls
import Logitune

Item {
    Column {
        anchors { left: parent.left; top: parent.top; margins: 32 }
        spacing: 24
        width: 400

        Text {
            text: "Settings"
            font { pixelSize: 22; bold: true }
            color: Theme.text
        }

        // Device info section
        Column {
            spacing: 12
            width: parent.width

            // Info rows
            Row {
                width: parent.width
                Text { text: "Device name"; width: 160; color: Theme.textSecondary; font.pixelSize: 13 }
                Text { text: DeviceModel.deviceName || "MX Master 3S"; color: Theme.text; font.pixelSize: 13 }
            }
            Row {
                width: parent.width
                Text { text: "Firmware version"; width: 160; color: Theme.textSecondary; font.pixelSize: 13 }
                Text { text: "Unknown"; color: Theme.text; font.pixelSize: 13 }
            }
            Row {
                width: parent.width
                Text { text: "Serial number"; width: 160; color: Theme.textSecondary; font.pixelSize: 13 }
                Text { text: DeviceModel.deviceSerial; color: Theme.text; font.pixelSize: 13 }
            }
            Row {
                width: parent.width
                Text { text: "Per-app profiles"; width: 160; color: Theme.textSecondary; font.pixelSize: 13 }
                Text { text: "Active"; color: Theme.batteryGreen; font.pixelSize: 13 }
            }

            // Separator
            Rectangle { width: parent.width; height: 1; color: Theme.border }

            // Dark mode toggle
            Row {
                width: parent.width
                Text {
                    text: "Dark mode"
                    font.pixelSize: 13
                    color: Theme.text
                    width: parent.width - darkToggle.width
                    anchors.verticalCenter: parent.verticalCenter
                }
                LogituneToggle {
                    id: darkToggle
                    checked: Theme.dark
                    onToggled: function(val) {
                        Theme.dark = val
                        SettingsModel.saveThemeDark(val)
                    }
                }
            }

            // Separator
            Rectangle { width: parent.width; height: 1; color: Theme.border }

            // Debug logging toggle
            Row {
                width: parent.width
                Text {
                    text: "Debug logging"
                    font.pixelSize: 13
                    color: Theme.text
                    width: parent.width - loggingToggle.width
                    anchors.verticalCenter: parent.verticalCenter
                }
                LogituneToggle {
                    id: loggingToggle
                    checked: SettingsModel.loggingEnabled
                    onToggled: function(val) { SettingsModel.loggingEnabled = val }
                }
            }

            // Log file path (visible when logging enabled)
            Text {
                visible: SettingsModel.loggingEnabled
                text: SettingsModel.logFilePath || ""
                font.pixelSize: 11
                color: Theme.textSecondary
                wrapMode: Text.WrapAnywhere
                width: parent.width
            }

            // Report Bug button (disabled when logging off)
            Rectangle {
                width: 180; height: 40
                radius: 4
                color: SettingsModel.loggingEnabled
                    ? (reportHover.hovered ? Theme.accent : "transparent")
                    : "transparent"
                border.color: SettingsModel.loggingEnabled ? Theme.accent : Theme.border
                border.width: 1
                opacity: SettingsModel.loggingEnabled ? 1.0 : 0.4

                Text {
                    anchors.centerIn: parent
                    text: "Report Bug"
                    font.pixelSize: 13
                    color: SettingsModel.loggingEnabled
                        ? (reportHover.hovered ? "#000000" : Theme.accent)
                        : Theme.textSecondary
                }

                HoverHandler { id: reportHover; enabled: SettingsModel.loggingEnabled }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: SettingsModel.loggingEnabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                    enabled: SettingsModel.loggingEnabled
                    onClicked: SettingsModel.openBugReport()
                }
            }

            // Test crash button (debug builds only)
            Rectangle {
                visible: SettingsModel.loggingEnabled
                width: 180; height: 40
                radius: 4; color: "transparent"
                border.color: "#ff4444"; border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "Test Exception"
                    font.pixelSize: 13; color: "#ff4444"
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: SettingsModel.testCrash()
                }
            }

            // Separator
            Rectangle { width: parent.width; height: 1; color: Theme.border }

            // Reset button
            Rectangle {
                width: 180; height: 40
                radius: 4; color: "transparent"
                border.color: Theme.border; border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "Reset to defaults"
                    font.pixelSize: 13; color: Theme.textSecondary
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: resetDialog.open()
                }
            }
        }
    }

    // Confirmation dialog
    Dialog {
        id: resetDialog
        title: "Reset to defaults?"
        modal: true
        anchors.centerIn: parent
        standardButtons: Dialog.Ok | Dialog.Cancel
    }
}
