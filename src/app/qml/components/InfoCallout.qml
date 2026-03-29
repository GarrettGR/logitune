import QtQuick
import QtQuick.Layouts
import Logitune

// Callout card for Point & Scroll page — matches ButtonCallout style.
// Shows a title and summary settings lines. Click to open DetailPanel.
Item {
    id: root

    // ── Public API ──────────────────────────────────────────────────────────
    property string title: ""
    property var    settings: []     // list of strings shown as lines
    property string calloutType: ""  // "scrollwheel" | "thumbwheel" | "pointerspeed"

    signal calloutClicked(string type)

    implicitWidth:  card.implicitWidth
    implicitHeight: card.implicitHeight

    // ── Card ────────────────────────────────────────────────────────────────
    Rectangle {
        id: card
        implicitWidth:  Math.min(Math.max(contentCol.implicitWidth + 24, 160), 220)
        implicitHeight: contentCol.implicitHeight + 18
        radius: 8
        color: Theme.cardBg
        border.color: Theme.cardBorder
        border.width: 1

        // Drop shadow
        Rectangle {
            x: 4; y: 4
            width: parent.width; height: parent.height
            radius: parent.radius
            color: Qt.rgba(0, 0, 0, 0.1)
            z: -1
        }

        Column {
            id: contentCol
            x: 12
            y: 9
            spacing: 2

            // Title (bold, accent on hover)
            Text {
                text: root.title
                font.pixelSize: 12
                font.weight: Font.DemiBold
                color: hoverHandler.hovered ? Theme.accent : Theme.text
                width: Math.min(implicitWidth, 196)
                elide: Text.ElideRight

                Behavior on color { ColorAnimation { duration: 150 } }
            }

            // Settings lines
            Repeater {
                model: root.settings
                delegate: Text {
                    text: modelData
                    font.pixelSize: 10
                    color: Theme.textSecondary
                    width: Math.min(implicitWidth, 196)
                    elide: Text.ElideRight
                }
            }
        }

        // Hover overlay
        Rectangle {
            anchors.fill: parent
            radius: card.radius
            color: hoverHandler.hovered ? Qt.rgba(0, 0, 0, 0.04) : "transparent"
            Behavior on color { ColorAnimation { duration: 100 } }
        }

        HoverHandler { id: hoverHandler }

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: root.calloutClicked(root.calloutType)
        }
    }
}
