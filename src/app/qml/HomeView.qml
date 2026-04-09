import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Logitune

Item {
    id: root
    signal deviceClicked()
    signal settingsClicked()

    // ── Top bar — 18.5vh max 144px ──────────────────────────────────────────
    RowLayout {
        id: topBar
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: Math.min(root.height * 0.185, 144)
        spacing: 12

        Item { width: 24 }  // left margin

        Text {
            id: greeting
            text: {
                var hour = new Date().getHours();
                if (hour >= 5 && hour < 12) return "Good Morning";
                if (hour >= 12 && hour < 17) return "Good Afternoon";
                return "Good Evening";
            }
            // Responsive: calc(1vw + 2.8vh) equivalent, clamped 24–36px
            font.pixelSize: Math.max(24, Math.min(36, root.width * 0.01 + root.height * 0.028))
            font.bold: true
            font.family: "Inter, sans-serif"
            color: Theme.text
        }

        Item { Layout.fillWidth: true }

        Text {
            id: settingsGear
            text: "\u2699"
            font.pixelSize: 20
            color: settingsHover.hovered ? Theme.accent : "#999999"

            HoverHandler { id: settingsHover }
            Behavior on color { ColorAnimation { duration: 150 } }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.settingsClicked()
            }
        }

        Item { width: 24 }  // right margin
    }

    // ── Center content ────────────────────────────────────────────────────────
    Item {
        anchors {
            top: topBar.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }

        // Device connected state
        Column {
            anchors.centerIn: parent
            spacing: 16
            visible: DeviceModel.deviceConnected

            // Mouse device image
            Image {
                id: deviceCard
                width: 200
                height: 296
                anchors.horizontalCenter: parent.horizontalCenter
                source: DeviceModel.frontImage
                fillMode: Image.PreserveAspectFit
                smooth: true
                mipmap: true

                HoverHandler { id: cardHover }

                scale: cardHover.hovered ? 1.03 : 1.0
                Behavior on scale { NumberAnimation { duration: 150; easing.type: Easing.OutCubic } }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.deviceClicked()
                }
            }

            // Device name label below the card
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: DeviceModel.deviceName || "MX Master 3S"
                font.pixelSize: 15
                font.bold: true
                color: Theme.text
            }

            BatteryChip {
                anchors.horizontalCenter: parent.horizontalCenter
                visible: DeviceModel.deviceConnected
            }
        }

        // No device connected state
        Column {
            anchors.centerIn: parent
            spacing: 12
            visible: !DeviceModel.deviceConnected

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "CONNECT YOUR DEVICE(S)"
                font.pixelSize: 14
                font.letterSpacing: 1.5
                font.bold: true
                color: Theme.textSecondary
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                text: "Connect a Logitech device to get started"
                font.pixelSize: 13
                color: "#BBBBBB"
            }
        }

        // GNOME AppIndicator notification banner
        Rectangle {
            id: trayBanner
            anchors { bottom: parent.bottom; left: parent.left; right: parent.right; margins: 16 }
            height: bannerCol.implicitHeight + 24
            radius: 8
            color: Theme.surface
            border.color: Theme.border
            border.width: 1
            visible: {
                var status = DeviceModel.gnomeTrayStatus()
                return status === "not-installed" || status === "disabled"
            }

            Column {
                id: bannerCol
                anchors { fill: parent; margins: 12 }
                spacing: 6

                Text {
                    text: {
                        var status = DeviceModel.gnomeTrayStatus()
                        if (status === "not-installed")
                            return "Tray icon requires the AppIndicator GNOME extension"
                        if (status === "disabled")
                            return "AppIndicator extension is installed but not enabled"
                        return ""
                    }
                    font.pixelSize: 12
                    font.bold: true
                    color: Theme.text
                    wrapMode: Text.WordWrap
                    width: parent.width
                }

                Text {
                    text: {
                        var status = DeviceModel.gnomeTrayStatus()
                        if (status === "not-installed")
                            return "Run: sudo pacman -S gnome-shell-extension-appindicator\nThen log out and back in."
                        if (status === "disabled")
                            return "Run: gnome-extensions enable appindicatorsupport@rgcjonas.gmail.com\nThen restart Logitune."
                        return ""
                    }
                    font.pixelSize: 11
                    font.family: "monospace"
                    color: Theme.textSecondary
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
            }

            // Dismiss button
            Text {
                anchors { top: parent.top; right: parent.right; margins: 8 }
                text: "\u2715"
                font.pixelSize: 14
                color: Theme.textSecondary
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: trayBanner.visible = false
                }
            }
        }
    }

}
