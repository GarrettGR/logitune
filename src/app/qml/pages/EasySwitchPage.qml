import QtQuick
import QtQuick.Controls

Item {
    id: root

    // Background
    Rectangle {
        anchors.fill: parent
        color: "#FFFFFF"
        radius: 0
    }

    Column {
        anchors.centerIn: parent
        spacing: 24

        Text {
            text: "Easy-Switch"
            font { pixelSize: 22; bold: true }
            color: "#222425"
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Text {
            text: "Switch between up to 3 devices"
            font.pixelSize: 13
            color: "#999999"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Row {
            spacing: 16
            anchors.horizontalCenter: parent.horizontalCenter

            // 3 channel cards
            Repeater {
                model: 3
                delegate: Rectangle {
                    width: 160; height: 120
                    radius: 12
                    color: index === 0 ? Qt.rgba(0.506, 0.306, 0.980, 0.06) : "#FFFFFF"
                    border.color: index === 0 ? "#814EFA" : "#F0F0F0"
                    border.width: index === 0 ? 3 : 1

                    Column {
                        anchors.centerIn: parent
                        spacing: 8

                        Text {
                            text: (index + 1).toString()
                            font { pixelSize: 24; bold: true }
                            color: index === 0 ? "#814EFA" : "#999999"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Text {
                            text: index === 0 ? "Active" : "Available"
                            font.pixelSize: 11
                            color: index === 0 ? "#814EFA" : "#999999"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Text {
                            text: index === 0 ? "Bluetooth" : "\u2014"
                            font.pixelSize: 10
                            color: "#AAAAAA"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }
            }
        }

        Text {
            text: "Press the Easy-Switch button on your mouse to change channels"
            font.pixelSize: 11
            color: "#999999"
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
