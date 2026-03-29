import QtQuick
import QtQuick.Controls
import Logitune

Item {
    id: root

    Rectangle {
        anchors.fill: parent
        color: Theme.background
    }

    // Scale content to fit available height
    readonly property real availH: height
    // Total fixed content: title(~60) + channels(~180) + footer(~30) + spacing(~80) = ~350
    readonly property real imageMaxH: Math.max(120, availH - 380)
    readonly property real imageH: Math.min(380, imageMaxH)
    readonly property real imageW: imageH * 0.676  // back image aspect ratio 692/1024

    Flickable {
        anchors.fill: parent
        contentHeight: contentCol.implicitHeight + 40
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: contentCol
            width: parent.width
            spacing: 20

            Item { width: 1; height: 10 }  // top padding

            // ── Device image with pulsing LED ───────────────────────────────
            Item {
                id: imageContainer
                width: root.imageW
                height: root.imageH
                anchors.horizontalCenter: parent.horizontalCenter

                Image {
                    id: deviceImage
                    anchors.fill: parent
                    source: DeviceModel.backImage
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    mipmap: true
                }

                readonly property real imgX: (width - deviceImage.paintedWidth) / 2
                readonly property real imgY: (height - deviceImage.paintedHeight) / 2
                readonly property real imgW: deviceImage.paintedWidth
                readonly property real imgH: deviceImage.paintedHeight

                readonly property var slotPositions: [
                    { x: 0.325, y: 0.658 },
                    { x: 0.384, y: 0.642 },
                    { x: 0.443, y: 0.643 }
                ]

                Repeater {
                    model: DeviceModel.easySwitchSlots
                    Rectangle {
                        required property int index
                        readonly property bool isActive: (index + 1) === DeviceModel.activeSlot
                        readonly property var pos: index < imageContainer.slotPositions.length
                            ? imageContainer.slotPositions[index] : { x: 0.5, y: 0.65 }

                        x: imageContainer.imgX + imageContainer.imgW * pos.x - width / 2
                        y: imageContainer.imgY + imageContainer.imgH * pos.y - height / 2
                        width: 7; height: 7; radius: 3.5
                        color: isActive ? Theme.accent : "transparent"
                        visible: isActive

                        SequentialAnimation on opacity {
                            running: isActive
                            loops: Animation.Infinite
                            NumberAnimation { to: 0.3; duration: 800; easing.type: Easing.InOutSine }
                            NumberAnimation { to: 1.0; duration: 800; easing.type: Easing.InOutSine }
                        }
                    }
                }
            }

            // ── Title ───────────────────────────────────────────────────────
            Column {
                spacing: 4
                anchors.horizontalCenter: parent.horizontalCenter

                Text {
                    text: "Easy-Switch"
                    font { pixelSize: 22; bold: true }
                    color: Theme.text
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Text {
                    text: "Connected via " + DeviceModel.connectionType
                    font.pixelSize: 13
                    color: Theme.accent
                    anchors.horizontalCenter: parent.horizontalCenter
                    visible: DeviceModel.deviceConnected
                }
            }

            // ── Channel list ────────────────────────────────────────────────
            Column {
                width: Math.min(380, root.width - 40)
                spacing: 0
                anchors.horizontalCenter: parent.horizontalCenter

                Repeater {
                    model: DeviceModel.easySwitchSlots
                    delegate: Rectangle {
                        required property int index
                        readonly property bool isActive: (index + 1) === DeviceModel.activeSlot

                        width: parent.width
                        height: 56
                        color: isActive ? Qt.rgba(Theme.accent.r, Theme.accent.g, Theme.accent.b, 0.06)
                                        : "transparent"
                        radius: 8

                        Row {
                            anchors {
                                fill: parent
                                leftMargin: 16
                                rightMargin: 16
                            }
                            spacing: 14

                            Rectangle {
                                width: 32; height: 32; radius: 16
                                anchors.verticalCenter: parent.verticalCenter
                                color: isActive ? Theme.accent : Theme.inputBg

                                Text {
                                    anchors.centerIn: parent
                                    text: (index + 1).toString()
                                    font { pixelSize: 14; bold: true }
                                    color: isActive ? Theme.activeTabText : Theme.textSecondary
                                }
                            }

                            Column {
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 2

                                Text {
                                    text: isActive ? DeviceModel.connectionType : "Available"
                                    font { pixelSize: 14; bold: isActive }
                                    color: isActive ? Theme.text : Theme.textSecondary
                                }
                                Text {
                                    text: isActive ? "Connected" : ""
                                    font.pixelSize: 11
                                    color: Theme.accent
                                }
                            }
                        }

                        Rectangle {
                            anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
                            anchors.leftMargin: 62
                            height: 1
                            color: Theme.border
                            visible: index < DeviceModel.easySwitchSlots - 1
                        }
                    }
                }
            }

            Text {
                text: "Press the Easy-Switch button on your mouse to change channels"
                font.pixelSize: 11
                color: Theme.textSecondary
                anchors.horizontalCenter: parent.horizontalCenter
            }

            Item { width: 1; height: 10 }  // bottom padding
        }
    }
}
