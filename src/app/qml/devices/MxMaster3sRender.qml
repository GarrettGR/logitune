import QtQuick
import Logitune

// MX Master 3S device render — mouse PNG with clickable button zones and hotspot circles.
Item {
    id: root

    implicitWidth:  280
    implicitHeight: 414

    signal buttonClicked(int buttonId)

    // "front" | "side" | "back" — page picks which view to show
    property string view: "front"
    property bool showHotspots: true  // set false on Point & Scroll page

    readonly property var _sources: ({
        "front": "qrc:/Logitune/qml/assets/mx-master-3s.png",
        "side":  "qrc:/Logitune/qml/assets/mx-master-3s-side.png",
        "back":  "qrc:/Logitune/qml/assets/mx-master-3s-back.png"
    })

    // Painted-rect properties — actual rendered area after PreserveAspectFit
    readonly property real paintedX: (width - mouseImage.paintedWidth) / 2
    readonly property real paintedY: (height - mouseImage.paintedHeight) / 2
    readonly property real paintedW: mouseImage.paintedWidth
    readonly property real paintedH: mouseImage.paintedHeight

    Image {
        id: mouseImage
        anchors.centerIn: parent
        width: parent.implicitWidth
        height: parent.implicitHeight
        source: root._sources[root.view] || root._sources["front"]
        fillMode: Image.PreserveAspectFit
        smooth: true
        mipmap: true
    }

    // Button IDs:
    //   2 = Middle / scroll wheel click
    //   3 = Back
    //   4 = Forward
    //   5 = Gesture button
    //   6 = Top / ModeShift
    //   7 = ThumbWheel (horizontal scroll)
    readonly property var hotspotPositions: [
        { buttonId: 2, dotXPct: 0.71, dotYPct: 0.15,
          zoneX: 0.59, zoneY: 0.05, zoneW: 0.22, zoneH: 0.15 },
        { buttonId: 3, dotXPct: 0.45, dotYPct: 0.60,
          zoneX: 0.32, zoneY: 0.52, zoneW: 0.22, zoneH: 0.14 },
        { buttonId: 4, dotXPct: 0.35, dotYPct: 0.43,
          zoneX: 0.22, zoneY: 0.36, zoneW: 0.22, zoneH: 0.14 },
        { buttonId: 5, dotXPct: 0.08, dotYPct: 0.58,
          zoneX: 0.00, zoneY: 0.50, zoneW: 0.16, zoneH: 0.15 },
        { buttonId: 6, dotXPct: 0.81, dotYPct: 0.34,
          zoneX: 0.70, zoneY: 0.26, zoneW: 0.18, zoneH: 0.14 },
        { buttonId: 7, dotXPct: 0.55, dotYPct: 0.515,
          zoneX: 0.44, zoneY: 0.44, zoneW: 0.22, zoneH: 0.14 },
    ]

    Repeater {
        model: root.showHotspots ? root.hotspotPositions.length : 0

        Item {
            required property int modelData
            readonly property var hp: root.hotspotPositions[modelData]

            MouseArea {
                x: root.paintedX + hp.zoneX * root.paintedW
                y: root.paintedY + hp.zoneY * root.paintedH
                width:  hp.zoneW * root.paintedW
                height: hp.zoneH * root.paintedH
                cursorShape: Qt.PointingHandCursor
                onClicked: root.buttonClicked(hp.buttonId)
            }

            Rectangle {
                x: root.paintedX + hp.dotXPct * root.paintedW  - 9
                y: root.paintedY + hp.dotYPct * root.paintedH - 9
                width: 18; height: 18
                radius: 9
                color: "transparent"
                border.color: Theme.accent
                border.width: 2
                opacity: 0.7

                Behavior on opacity { NumberAnimation { duration: 200 } }

                Rectangle {
                    anchors.centerIn: parent
                    width: 6; height: 6
                    radius: 3
                    color: Theme.accent
                    opacity: 0.6
                }
            }
        }
    }
}
