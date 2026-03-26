import QtQuick
import Logitune

Row {
    spacing: 8
    height: 40

    Repeater {
        model: ProfileModel
        delegate: Rectangle {
            width: 56
            height: 40
            radius: 8
            color: model.isActive ? Qt.rgba(0.506, 0.306, 0.980, 0.06) : "transparent"
            border.color: model.isActive ? "#814EFA" : "transparent"
            border.width: model.isActive ? 2 : 0

            Column {
                anchors.centerIn: parent
                spacing: 2

                Text {
                    text: model.icon || "\u229E"
                    font.pixelSize: 16
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                Text {
                    text: model.name
                    font.pixelSize: 8
                    color: "#999999"
                    anchors.horizontalCenter: parent.horizontalCenter
                    visible: model.name.length <= 10
                }
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: ProfileModel.setActiveByIndex(model.index)
            }
        }
    }

    // Add button
    Rectangle {
        width: 40
        height: 40
        radius: 8
        color: "transparent"

        Text {
            anchors.centerIn: parent
            text: "+"
            font.pixelSize: 20
            color: "#CCCCCC"
        }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                // TODO: open add-app dialog
            }
        }
    }
}
