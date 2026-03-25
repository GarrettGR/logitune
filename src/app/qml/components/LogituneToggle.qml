import QtQuick

// Logitune toggle switch — purple ON, gray OFF
Item {
    id: root

    property bool   checked: false
    property string label:   ""

    signal toggled(bool checked)

    implicitWidth:  row.implicitWidth
    implicitHeight: row.implicitHeight

    Row {
        id: row
        spacing: 10

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: root.label
            font.pixelSize: 12
            color: "#444444"
            visible: root.label.length > 0
        }

        Rectangle {
            id: track
            width:  40
            height: 22
            radius: 11
            color:  root.checked ? "#7B61FF" : "#CCCCCC"

            Behavior on color { ColorAnimation { duration: 150 } }

            Rectangle {
                id: knob
                width:  18
                height: 18
                radius: 9
                color:  "#FFFFFF"
                anchors.verticalCenter: parent.verticalCenter
                x: root.checked ? parent.width - width - 2 : 2

                Behavior on x { NumberAnimation { duration: 150; easing.type: Easing.InOutQuad } }
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    root.checked = !root.checked
                    root.toggled(root.checked)
                }
            }
        }
    }
}
