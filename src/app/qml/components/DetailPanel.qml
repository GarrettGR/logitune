import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Logitune

// Slide-in detail panel from the right edge.
// panelType: "scrollwheel" | "thumbwheel" | "pointerspeed"
Rectangle {
    id: root

    // ── Public API ──────────────────────────────────────────────────────────
    property string panelType: ""

    signal closeRequested

    // ── Geometry / Appearance — percentage-based width ────────────────────
    width: {
        var w = (parent ? parent.width : 960) * 0.33
        return Math.max(360, Math.min(w, 478))
    }
    color:  "#F5F5F5"
    radius: 12

    // Drop shadow illusion via a slightly larger, lighter rect behind
    layer.enabled: true

    // ── Slide-in animation ──────────────────────────────────────────────────
    property bool opened: false

    // x is controlled by the parent page — do not set it here
    Behavior on x {
        NumberAnimation { duration: 200; easing.type: Easing.OutQuad }
    }

    // ── Content ─────────────────────────────────────────────────────────────
    ColumnLayout {
        anchors {
            fill: parent
            margins: 32
        }
        spacing: 16

        // Header row: title + X button
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
                id: panelTitle
                text: {
                    if (root.panelType === "scrollwheel")   return "Scroll wheel"
                    if (root.panelType === "thumbwheel")    return "Thumb wheel"
                    if (root.panelType === "pointerspeed")  return "Pointer speed"
                    return ""
                }
                font.pixelSize: 16
                font.bold: true
                color: "#222425"
                Layout.fillWidth: true
            }

            // Close button
            Rectangle {
                width: 28; height: 28
                radius: 14
                color: closeHover.hovered ? "#E1E2E3" : "transparent"
                Behavior on color { ColorAnimation { duration: 100 } }

                Text {
                    anchors.centerIn: parent
                    text: "\u2715"   // ✕
                    font.pixelSize: 13
                    color: "#555555"
                }
                HoverHandler { id: closeHover }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: root.closeRequested()
                }
            }
        }

        // Thin divider
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#F0F0F0"
        }

        // ── Per-type content ────────────────────────────────────────────────
        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: {
                if (root.panelType === "scrollwheel")  return scrollWheelContent
                if (root.panelType === "thumbwheel")   return thumbWheelContent
                if (root.panelType === "pointerspeed") return pointerSpeedContent
                return null
            }
        }
    }

    // ── Scroll Wheel content ────────────────────────────────────────────────
    Component {
        id: scrollWheelContent

        Column {
            width: parent.width
            spacing: 20

            // Speed slider
            LogituneSlider {
                width: parent.width
                label: "Speed"
                value: 50
            }

            // Scroll direction radio group
            Column {
                width: parent.width
                spacing: 8

                Text {
                    text: "Scroll direction"
                    font.pixelSize: 16
                    font.bold: true
                    color: "#444444"
                    bottomPadding: 14
                }

                ButtonGroup { id: directionGroup }

                RadioButton {
                    id: naturalRadio
                    text: "Natural"
                    checked: true
                    ButtonGroup.group: directionGroup

                    contentItem: Text {
                        leftPadding: naturalRadio.indicator.width + naturalRadio.spacing
                        text: naturalRadio.text
                        font.pixelSize: 12
                        color: "#222425"
                        verticalAlignment: Text.AlignVCenter
                    }

                    indicator: Rectangle {
                        implicitWidth:  18
                        implicitHeight: 18
                        radius: 9
                        border.color: naturalRadio.checked ? "#814EFA" : "#AAAAAA"
                        border.width: 2
                        color: "transparent"
                        anchors.verticalCenter: parent.verticalCenter

                        Rectangle {
                            anchors.centerIn: parent
                            width: 8; height: 8
                            radius: 4
                            color: "#814EFA"
                            visible: naturalRadio.checked
                        }
                    }
                }

                RadioButton {
                    id: standardRadio
                    text: "Standard"
                    ButtonGroup.group: directionGroup

                    contentItem: Text {
                        leftPadding: standardRadio.indicator.width + standardRadio.spacing
                        text: standardRadio.text
                        font.pixelSize: 12
                        color: "#222425"
                        verticalAlignment: Text.AlignVCenter
                    }

                    indicator: Rectangle {
                        implicitWidth:  18
                        implicitHeight: 18
                        radius: 9
                        border.color: standardRadio.checked ? "#814EFA" : "#AAAAAA"
                        border.width: 2
                        color: "transparent"
                        anchors.verticalCenter: parent.verticalCenter

                        Rectangle {
                            anchors.centerIn: parent
                            width: 8; height: 8
                            radius: 4
                            color: "#814EFA"
                            visible: standardRadio.checked
                        }
                    }
                }
            }

            // Smooth scrolling toggle
            Row {
                width: parent.width

                Text {
                    text: "Smooth scrolling"
                    font.pixelSize: 16
                    font.bold: true
                    color: "#444444"
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - smoothToggle.width
                }

                LogituneToggle {
                    id: smoothToggle
                    checked: false
                }
            }

            // SmartShift toggle — padding: 12px 32px 24px
            Column {
                width: parent.width
                topPadding: 12
                bottomPadding: 24
                spacing: 12

                Row {
                    width: parent.width

                    Text {
                        text: "SmartShift"
                        font.pixelSize: 16
                        font.bold: true
                        color: "#444444"
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - smartShiftToggle.width
                    }

                    LogituneToggle {
                        id: smartShiftToggle
                        checked: true
                    }
                }

                // SmartShift sensitivity slider — only shown when SmartShift is on
                LogituneSlider {
                    width: parent.width
                    label: "SmartShift sensitivity"
                    value: 30
                    visible: smartShiftToggle.checked

                    Behavior on opacity { NumberAnimation { duration: 150 } }
                    opacity: smartShiftToggle.checked ? 1.0 : 0.0
                }
            }
        }
    }

    // ── Thumb Wheel content ─────────────────────────────────────────────────
    Component {
        id: thumbWheelContent

        Column {
            width: parent.width
            spacing: 20

            // Speed slider
            LogituneSlider {
                width: parent.width
                label: "Speed"
                value: 50
            }

            // Direction invert toggle
            Row {
                width: parent.width

                Text {
                    text: "Invert direction"
                    font.pixelSize: 16
                    font.bold: true
                    color: "#444444"
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - invertToggle.width
                }

                LogituneToggle {
                    id: invertToggle
                    checked: false
                }
            }
        }
    }

    // ── Pointer Speed content ───────────────────────────────────────────────
    Component {
        id: pointerSpeedContent

        Column {
            width: parent.width
            spacing: 16

            // Detect KDE — heuristic: check XDG_CURRENT_DESKTOP env
            property bool isKDE: (Qt.platform.os === "linux") &&
                                  (Qt.application.name.length > 0) // placeholder; real check via C++ env

            LogituneSlider {
                id: speedSlider
                width: parent.width
                label: "Pointer speed"
                value: 50
                // Dim slider when not on KDE — tooltip provided separately
                opacity: parent.isKDE ? 1.0 : 0.4
            }

            // "Requires KDE Plasma" notice when greyed out
            Rectangle {
                width: parent.width
                height: kdeNote.implicitHeight + 16
                radius: 4
                color: "#FFF3CD"
                border.color: "#FFD97D"
                border.width: 1
                visible: !parent.isKDE

                Text {
                    id: kdeNote
                    anchors {
                        left: parent.left
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                        leftMargin: 12
                        rightMargin: 12
                    }
                    text: "Pointer speed control requires KDE Plasma."
                    font.pixelSize: 11
                    color: "#856404"
                    wrapMode: Text.WordWrap
                }
            }
        }
    }
}
