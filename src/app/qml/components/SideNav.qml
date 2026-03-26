import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Logitune

Rectangle {
    id: sideNav
    color: "#FFFFFF"

    signal pageSelected(string pageName)
    property string currentPage: "buttons"

    // Nav items model
    readonly property var navItems: [
        { name: "buttons",     label: "BUTTONS",         icon: "\uD83C\uDFAE", enabled: true  },
        { name: "pointscroll", label: "POINT & SCROLL",  icon: "\u25CE",       enabled: true  },
        { name: "easyswitch",  label: "EASY-SWITCH",     icon: "\u21C4",       enabled: true  },
        { name: "flow",        label: "FLOW",             icon: "\u2194",       enabled: false },
        { name: "settings",    label: "SETTINGS",         icon: "\u2261",       enabled: true  }
    ]

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Top spacer
        Item { height: 16 }

        // Device name area at the top of the sidebar
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 35
            Layout.rightMargin: 16
            spacing: 8

            Text {
                text: DeviceModel.deviceName || "MX Master 3S"
                font.pixelSize: 13
                font.bold: true
                color: "#222425"
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
        }

        // Divider
        Rectangle {
            Layout.fillWidth: true
            Layout.topMargin: 12
            Layout.bottomMargin: 4
            height: 1
            color: "#F0F0F0"
        }

        // Nav items
        Repeater {
            model: sideNav.navItems

            delegate: Item {
                Layout.fillWidth: true
                Layout.topMargin: index > 0 ? 17 : 0    // 17px vertical margin between tabs
                height: 40
                opacity: modelData.enabled ? 1.0 : 0.4

                Rectangle {
                    id: pill
                    anchors {
                        fill: parent
                        leftMargin: 12
                        rightMargin: 8
                    }
                    radius: 4
                    color: sideNav.currentPage === modelData.name
                           ? "transparent"
                           : (itemHover.hovered && modelData.enabled ? "#F5F5F5" : "transparent")
                    border.color: sideNav.currentPage === modelData.name ? "#6039B2" : "transparent"
                    border.width: sideNav.currentPage === modelData.name ? 1 : 0

                    // Subtle drop shadow for active tab
                    layer.enabled: sideNav.currentPage === modelData.name
                    layer.effect: Item {}

                    Behavior on color { ColorAnimation { duration: 200 } }
                    Behavior on border.color { ColorAnimation { duration: 200 } }

                    RowLayout {
                        anchors {
                            fill: parent
                            leftMargin: 7
                            rightMargin: 12
                            topMargin: 0
                            bottomMargin: 0
                        }
                        spacing: 10

                        // 32x32 icon container
                        Item {
                            width: 32; height: 32
                            Text {
                                anchors.centerIn: parent
                                text: modelData.icon
                                font.pixelSize: 18
                                color: sideNav.currentPage === modelData.name ? "#814EFA" : "#555555"
                                Behavior on color { ColorAnimation { duration: 200 } }
                            }
                        }

                        Text {
                            text: modelData.label
                            font.pixelSize: 13
                            font.letterSpacing: 0.6
                            font.bold: true
                            color: sideNav.currentPage === modelData.name ? "#814EFA" : "#444444"
                            // Strikethrough when disabled
                            font.strikeout: !modelData.enabled
                            Layout.fillWidth: true

                            Behavior on color { ColorAnimation { duration: 200 } }
                        }
                    }
                }

                HoverHandler { id: itemHover }

                MouseArea {
                    anchors.fill: parent
                    enabled: modelData.enabled
                    cursorShape: modelData.enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                    onClicked: {
                        sideNav.currentPage = modelData.name
                        sideNav.pageSelected(modelData.name)
                    }
                }
            }
        }

        // Bottom fill
        Item { Layout.fillHeight: true }
    }
}
