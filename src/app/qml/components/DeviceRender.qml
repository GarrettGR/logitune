import QtQuick

// Mouse device render — MX Master 3S PNG with invisible clickable button zones overlaid.
Item {
    id: root

    implicitWidth:  220
    implicitHeight: 326

    signal buttonClicked(int buttonId)

    // ── Mouse image ──────────────────────────────────────────────────────────
    Image {
        id: mouseImage
        anchors.centerIn: parent
        width: parent.implicitWidth
        height: parent.implicitHeight
        source: "qrc:/Logitune/qml/assets/mx-master-3s.png"
        fillMode: Image.PreserveAspectFit
        smooth: true
        mipmap: true
    }

    // ── Button zone overlays with hotspot circles ────────────────────────────
    // Positions tuned for the 220x326 MX Master 3S render at 3/4 angle.
    // Each zone has a 16x16 hotspot circle (2px border, full radius).

    readonly property var hotspotPositions: [
        // 0: Left click centre
        { zoneX: 20, zoneY: 10, zoneW: 90, zoneH: 130, dotX: 55,  dotY: 70  },
        // 1: Right click centre
        { zoneX: 115, zoneY: 10, zoneW: 90, zoneH: 130, dotX: 160, dotY: 70  },
        // 2: Middle / scroll wheel
        { zoneX: 85, zoneY: 40, zoneW: 40, zoneH: 70,  dotX: 105, dotY: 75  },
        // 3: Back (thumb rear)
        { zoneX: 0, zoneY: 195, zoneW: 45, zoneH: 40,  dotX: 15,  dotY: 215 },
        // 4: Forward (thumb front)
        { zoneX: 0, zoneY: 150, zoneW: 45, zoneH: 40,  dotX: 15,  dotY: 170 },
        // 5: Thumb / gesture
        { zoneX: 0, zoneY: 170, zoneW: 40, zoneH: 30,  dotX: 12,  dotY: 185 },
        // 6: Top button (behind scroll)
        { zoneX: 150, zoneY: 30, zoneW: 40, zoneH: 25,  dotX: 170, dotY: 42  }
    ]

    Repeater {
        model: 7

        Item {
            required property int modelData

            // Invisible hit area
            MouseArea {
                x: root.hotspotPositions[modelData].zoneX
                y: root.hotspotPositions[modelData].zoneY
                width:  root.hotspotPositions[modelData].zoneW
                height: root.hotspotPositions[modelData].zoneH
                cursorShape: Qt.PointingHandCursor
                onClicked: root.buttonClicked(modelData)
            }

            // 16x16 hotspot circle
            Rectangle {
                x: root.hotspotPositions[modelData].dotX - 8
                y: root.hotspotPositions[modelData].dotY - 8
                width: 16; height: 16
                radius: 8
                color: "transparent"
                border.color: "#814EFA"
                border.width: 2
                opacity: 0.6

                Behavior on opacity { NumberAnimation { duration: 200 } }

                // Pulse on hover (subtle)
                Rectangle {
                    anchors.centerIn: parent
                    width: 6; height: 6
                    radius: 3
                    color: "#814EFA"
                    opacity: 0.5
                }
            }
        }
    }
}
