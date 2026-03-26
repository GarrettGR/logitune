import QtQuick
import QtQuick.Layouts
import Logitune

// ─────────────────────────────────────────────────────────────────────────────
// ButtonsPage — main button-remapping screen.
//
// Layout:
//   Left area: DeviceRender (mouse SVG) + ButtonCallout[7] cards
//   Right area: ActionsPanel (slides in when a button is selected)
// ─────────────────────────────────────────────────────────────────────────────
Item {
    id: root

    // Currently selected button (-1 = none)
    property int selectedButton: -1

    // ── Callout layout data ────────────────────────────────────────────────
    // Each entry: { dx, dy } offset from mouse render centre.
    // Positive dx = right, positive dy = down.
    // The mouse render is 200×280, centred at (100, 140) relative to itself.
    readonly property var calloutLayout: [
        // 0: Left click — top-left
        { dx: -180, dy: -80, lineX: -50, lineY:  -70 },
        // 1: Right click — top-right
        { dx:  130, dy: -80, lineX:  50, lineY:  -70 },
        // 2: Middle / scroll — top-centre
        { dx:  -30, dy: -140, lineX:   0, lineY:  -80 },
        // 3: Back — left side lower
        { dx: -180, dy:  110, lineX: -90, lineY:   50 },
        // 4: Forward — left side upper
        { dx: -180, dy:  -10, lineX: -90, lineY:  -40 },
        // 5: Thumb / gesture — left middle
        { dx: -180, dy:   50, lineX: -90, lineY:   -5 },
        // 6: Top (behind scroll) — right side upper
        { dx:  130, dy:   20, lineX:  70, lineY:  -70 },
    ]

    // Convenience: button display data (mirrors ButtonModel defaults)
    readonly property var buttonMeta: [
        { name: "Left click"  },
        { name: "Right click" },
        { name: "Middle click"},
        { name: "Back"        },
        { name: "Forward"     },
        { name: "Thumb"       },
        { name: "Top"         },
    ]

    // ── Background ─────────────────────────────────────────────────────────
    Rectangle {
        anchors.fill: parent
        color: "#FFFFFF"
    }

    // ── Dismiss panel by clicking the background ───────────────────────────
    MouseArea {
        anchors.fill: parent
        enabled: root.selectedButton >= 0
        onClicked: root.selectedButton = -1
    }

    // ── Centre area: mouse render + callouts ───────────────────────────────
    Item {
        id: renderArea
        anchors {
            left:   parent.left
            top:    parent.top
            bottom: parent.bottom
            right:  actionsPanel.left
        }

        // DeviceRender centred in the available space
        DeviceRender {
            id: deviceRender
            anchors.centerIn: parent

            onButtonClicked: function(buttonId) {
                root.selectedButton = buttonId
                // Sync ActionModel selection to current button's action
                var actionName = ButtonModel.actionNameForButton(buttonId)
                actionsPanel.buttonId           = buttonId
                actionsPanel.buttonName         = root.buttonMeta[buttonId].name
                actionsPanel.currentAction      = actionName
                actionsPanel.currentActionType  = ButtonModel.actionTypeForButton(buttonId)
            }
        }

        // ── Callout cards ──────────────────────────────────────────────────
        Repeater {
            id: calloutRepeater
            model: 7

            delegate: ButtonCallout {
                required property int modelData

                // Position relative to renderArea, offset from mouse centre
                x: renderArea.width  / 2 + root.calloutLayout[modelData].dx - width  / 2
                y: renderArea.height / 2 + root.calloutLayout[modelData].dy - height / 2

                // Line tip: centre of the mouse zone in renderArea coords
                lineToX: renderArea.width  / 2 + root.calloutLayout[modelData].lineX
                lineToY: renderArea.height / 2 + root.calloutLayout[modelData].lineY

                actionName: ButtonModel.actionNameForButton(modelData)
                buttonName: root.buttonMeta[modelData].name
                selected:   root.selectedButton === modelData

                onClicked: {
                    root.selectedButton = modelData
                    actionsPanel.buttonId          = modelData
                    actionsPanel.buttonName        = root.buttonMeta[modelData].name
                    actionsPanel.currentAction     = actionName
                    actionsPanel.currentActionType = ButtonModel.actionTypeForButton(modelData)
                }

                // Keep callout updated when ButtonModel data changes
                Connections {
                    target: ButtonModel
                    function onDataChanged() {
                        actionName = ButtonModel.actionNameForButton(modelData)
                    }
                }
            }
        }
    }

    // ── Actions Panel ──────────────────────────────────────────────────────
    ActionsPanel {
        id: actionsPanel

        anchors {
            top:    parent.top
            bottom: parent.bottom
        }

        // Slide in/out from right
        x: root.selectedButton >= 0
           ? parent.width - width
           : parent.width

        Behavior on x {
            NumberAnimation { duration: 300; easing.type: Easing.OutCubic }
        }

        onClosed: root.selectedButton = -1

        onActionSelected: function(actionName, actionType) {
            if (root.selectedButton >= 0) {
                ButtonModel.setAction(root.selectedButton, actionName, actionType)
                // Update all callouts via binding refresh
            }
        }
    }
}
