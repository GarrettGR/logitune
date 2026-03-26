import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Logitune

// Slide-in Actions panel from the right edge.
// Width: 33% of window, clamped 360–478px.
Rectangle {
    id: root

    // ── Public API ─────────────────────────────────────────────────────────
    property int    buttonId:     -1
    property string buttonName:   ""
    property string currentAction: ""
    property string currentActionType: ""

    signal closed()
    signal actionSelected(string actionName, string actionType)

    // ── Geometry — percentage-based width ──────────────────────────────────
    width: {
        var w = (parent ? parent.width : 960) * 0.33
        return Math.max(360, Math.min(w, 478))
    }
    color:  "#F5F5F5"
    radius: 0

    // Left border
    Rectangle {
        anchors { left: parent.left; top: parent.top; bottom: parent.bottom }
        width: 1
        color: "#F0F0F0"
    }

    // ── Slide animation ────────────────────────────────────────────────────
    // Controlled by parent via x property. No internal animation here;
    // parent drives x so it can coordinate with layout.

    // ── Content ────────────────────────────────────────────────────────────
    ColumnLayout {
        anchors {
            fill: parent
            topMargin:    16
            bottomMargin: 16
            leftMargin:   0
            rightMargin:  0
        }
        spacing: 0

        // Header row
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin:  20
            Layout.rightMargin: 16

            Column {
                spacing: 2

                Text {
                    text: "Actions"
                    font.pixelSize: 18
                    font.bold: true
                    color: "#222425"
                }

                Text {
                    text: root.buttonName.length > 0 ? root.buttonName : "Button"
                    font.pixelSize: 12
                    color: "#888888"
                }
            }

            Item { Layout.fillWidth: true }

            // Close button
            Rectangle {
                width: 28; height: 28
                radius: 14
                color: closeHover.hovered ? "#E8E8E8" : "transparent"
                Behavior on color { ColorAnimation { duration: 100 } }

                Text {
                    anchors.centerIn: parent
                    text: "\u00D7"
                    font.pixelSize: 18
                    color: "#999999"
                }

                HoverHandler { id: closeHover }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.closed()
                }
            }
        }

        // Divider
        Rectangle {
            Layout.fillWidth: true
            Layout.topMargin: 12
            height: 1
            color: "#F0F0F0"
        }

        // ── Search bar ───────────────────────────────────────────────────────
        Item {
            Layout.fillWidth: true
            Layout.leftMargin: 16
            Layout.rightMargin: 16
            height: 80

            Rectangle {
                anchors {
                    fill: parent
                    topMargin: 16
                    bottomMargin: 16
                }
                radius: 4
                color: "#FFFFFF"
                border.color: searchInput.activeFocus ? "#814EFA" : "#E1E2E3"
                border.width: 1

                Behavior on border.color { ColorAnimation { duration: 200 } }

                TextInput {
                    id: searchInput
                    anchors {
                        fill: parent
                        leftMargin: 12
                        rightMargin: 12
                    }
                    verticalAlignment: TextInput.AlignVCenter
                    font.pixelSize: 14
                    color: "#222425"
                    clip: true

                    Text {
                        anchors.fill: parent
                        verticalAlignment: Text.AlignVCenter
                        text: "Search actions..."
                        font.pixelSize: 14
                        color: "#AAAAAA"
                        visible: !searchInput.text && !searchInput.activeFocus
                    }
                }
            }
        }

        // ── SMART ACTIONS section ──────────────────────────────────────────
        Item {
            Layout.fillWidth: true
            Layout.leftMargin:  33
            Layout.rightMargin: 16
            Layout.topMargin:   12
            implicitHeight: smartHeader.implicitHeight + (smartExpanded ? smartBody.implicitHeight + 8 : 0)
            clip: true

            property bool smartExpanded: false

            RowLayout {
                id: smartHeader
                width: parent.width
                height: 50
                spacing: 6

                Text {
                    text: "SMART ACTIONS"
                    font.pixelSize: 14
                    font.letterSpacing: 0.8
                    font.bold: true
                    font.capitalization: Font.AllUppercase
                    color: "#888888"
                    Layout.fillWidth: true
                }

                Text {
                    text: parent.parent.smartExpanded ? "\u25B2" : "\u25BC"
                    font.pixelSize: 9
                    color: "#AAAAAA"
                }
            }

            Column {
                id: smartBody
                anchors { top: smartHeader.bottom; topMargin: 8; left: parent.left; right: parent.right }
                visible: parent.smartExpanded

                Text {
                    width: parent.width
                    text: "Create Smart Actions to assign\ncontext-aware behaviors to buttons."
                    font.pixelSize: 12
                    color: "#AAAAAA"
                    wrapMode: Text.Wrap
                    lineHeight: 1.5
                }
            }

            MouseArea {
                anchors { left: parent.left; right: parent.right; top: parent.top }
                height: smartHeader.height + 4
                cursorShape: Qt.PointingHandCursor
                onClicked: parent.smartExpanded = !parent.smartExpanded
            }
        }

        // Divider
        Rectangle {
            Layout.fillWidth: true
            Layout.topMargin: 8
            height: 1
            color: "#F0F0F0"
        }

        // ── OTHER ACTIONS section header ───────────────────────────────────
        Item {
            Layout.fillWidth: true
            Layout.leftMargin:  33
            Layout.rightMargin: 16
            Layout.topMargin:   10
            implicitHeight: 50

            RowLayout {
                width: parent.width
                height: 50

                Text {
                    text: "OTHER ACTIONS"
                    font.pixelSize: 14
                    font.letterSpacing: 0.8
                    font.bold: true
                    font.capitalization: Font.AllUppercase
                    color: "#888888"
                    Layout.fillWidth: true
                }
            }
        }

        // ── Action list ────────────────────────────────────────────────────
        ListView {
            id: actionList
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: 4
            clip: true
            model: ActionModel
            spacing: 0

            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

            delegate: Item {
                width:  actionList.width
                height: 48
                required property string name
                required property string description
                required property string actionType
                required property int    index

                readonly property bool isSelected: name === root.currentAction

                Rectangle {
                    anchors {
                        fill: parent
                        leftMargin:  16
                        rightMargin: 16
                        topMargin:   0
                        bottomMargin: 12
                    }
                    radius: 4
                    color: isSelected
                           ? "#814EFA"
                           : (rowHover.hovered ? "#EEEEFF" : "transparent")

                    Behavior on color { ColorAnimation { duration: 200 } }

                    RowLayout {
                        anchors {
                            fill: parent
                            leftMargin:  24
                            rightMargin: 16
                        }
                        spacing: 16

                        // 32x32 icon container with radio indicator
                        Item {
                            width: 32; height: 32

                            Rectangle {
                                anchors.centerIn: parent
                                width: 14; height: 14
                                radius: 7
                                color: isSelected ? "#FFFFFF" : "transparent"
                                border.color: isSelected ? "#FFFFFF" : "#BBBBBB"
                                border.width: 2

                                Rectangle {
                                    anchors.centerIn: parent
                                    width: 6; height: 6
                                    radius: 3
                                    color: isSelected ? "#814EFA" : "transparent"
                                }
                            }
                        }

                        Text {
                            text: name
                            font.pixelSize: 20
                            font.bold: true
                            color: isSelected ? "#FFFFFF" : "#222425"
                            Layout.fillWidth: true
                            elide: Text.ElideRight

                            Behavior on color { ColorAnimation { duration: 200 } }
                        }
                    }

                    HoverHandler { id: rowHover }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.currentAction = name
                            root.currentActionType = actionType
                            root.actionSelected(name, actionType)
                        }
                    }
                }
            }
        }

        // Divider above contextual area
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#F0F0F0"
            visible: root.currentActionType.length > 0
        }

        // ── Contextual controls (Loader) ───────────────────────────────────
        Loader {
            id: contextLoader
            Layout.fillWidth: true
            Layout.topMargin: 0
            Layout.bottomMargin: 4
            visible: item !== null

            sourceComponent: {
                switch (root.currentActionType) {
                case "keystroke":      return keystrokeComponent
                case "gesture-trigger": return gestureComponent
                default:               return descriptionComponent
                }
            }
        }
    }

    // ── Contextual component definitions ───────────────────────────────────

    Component {
        id: keystrokeComponent

        Item {
            implicitHeight: col.implicitHeight + 24
            width: parent ? parent.width : 340

            Column {
                id: col
                anchors {
                    left: parent.left; right: parent.right
                    top: parent.top
                    leftMargin: 20; rightMargin: 20; topMargin: 12
                }
                spacing: 8

                Text {
                    text: "Key combination"
                    font.pixelSize: 12
                    font.bold: true
                    color: "#444444"
                }

                KeystrokeCapture {
                    width: parent.width
                }
            }
        }
    }

    Component {
        id: gestureComponent

        Item {
            implicitHeight: gestureCol.implicitHeight + 24
            width: parent ? parent.width : 340

            Column {
                id: gestureCol
                anchors {
                    left: parent.left; right: parent.right; top: parent.top
                    leftMargin: 20; rightMargin: 20; topMargin: 12
                }
                spacing: 4

                Text {
                    text: "Gesture directions"
                    font.pixelSize: 12
                    font.bold: true
                    color: "#444444"
                    bottomPadding: 4
                }

                Repeater {
                    model: [
                        { dir: "\u2191", label: "Up" },
                        { dir: "\u2193", label: "Down" },
                        { dir: "\u2190", label: "Left" },
                        { dir: "\u2192", label: "Right" },
                        { dir: "\u25C9", label: "Click" },
                    ]

                    delegate: Rectangle {
                        width: parent.width
                        height: 36
                        radius: 4
                        color: "#FFFFFF"
                        border.color: "#F0F0F0"
                        border.width: 1

                        RowLayout {
                            anchors { fill: parent; leftMargin: 10; rightMargin: 10 }
                            spacing: 8

                            Text {
                                text: modelData.dir
                                font.pixelSize: 14
                                color: "#814EFA"
                            }
                            Text {
                                text: modelData.label
                                font.pixelSize: 12
                                color: "#444444"
                                Layout.fillWidth: true
                            }
                            Text {
                                text: "None"
                                font.pixelSize: 11
                                color: "#AAAAAA"
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: descriptionComponent

        Item {
            implicitHeight: descText.implicitHeight + 24
            width: parent ? parent.width : 340

            Text {
                id: descText
                anchors {
                    left: parent.left; right: parent.right; top: parent.top
                    leftMargin: 20; rightMargin: 20; topMargin: 12
                }
                text: {
                    switch (root.currentActionType) {
                    case "none":         return "This button will do nothing when pressed."
                    case "screenshot":   return "Captures the full screen and saves to your pictures folder."
                    case "show-desktop": return "Minimises all windows to reveal the desktop."
                    case "app-expose":   return "Shows all open windows for the current application."
                    case "calculator":   return "Opens the system calculator application."
                    case "mute":         return "Toggles your system audio mute."
                    case "media":        return "Controls media playback (play, pause, skip)."
                    case "volume-up":    return "Increases system volume by one step."
                    case "volume-down":  return "Decreases system volume by one step."
                    default:             return root.currentAction.length > 0
                                                ? root.currentAction + " action assigned."
                                                : ""
                    }
                }
                font.pixelSize: 12
                color: "#888888"
                wrapMode: Text.Wrap
                lineHeight: 1.5
            }
        }
    }
}
