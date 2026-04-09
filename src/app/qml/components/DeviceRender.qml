import QtQuick
import Logitune

// Device-agnostic render delegator. Loads the per-device QML component
// declared by the active device's descriptor (DeviceModel.renderComponent).
// Forwards `view`, `showHotspots` to the loaded item and routes its
// `buttonClicked` signal back up.
Loader {
    id: root

    // Loader automatically derives its implicit size from the loaded item.

    // Public API (replaces the previous imageSource — each device now
    // maps `view` to its own asset paths internally).
    property string view: "front"      // "front" | "side" | "back"
    property bool showHotspots: true

    signal buttonClicked(int buttonId)

    // Forward painted-rect properties from the loaded render item so pages
    // can still reference deviceRender.paintedX/Y/W/H directly.
    readonly property real paintedX: item ? item.paintedX : 0
    readonly property real paintedY: item ? item.paintedY : 0
    readonly property real paintedW: item ? item.paintedW : 0
    readonly property real paintedH: item ? item.paintedH : 0

    // DeviceModel.renderComponent returns a basename like "MxMaster3sRender".
    // On Qt 6.5+ the file is at qml/devices/<name>.qml via QTP0004.
    // On Qt 6.4 the file is aliased to qml/<name>.qml (see CMakeLists).
    // Use the flat path on Qt 6.4 and the subdirectory path on Qt 6.5+.
    readonly property string _baseName: DeviceModel.renderComponent || ""
    readonly property bool _qt65plus: {
        var v = Qt.qtVersion ? Qt.qtVersion.split(".") : ["6", "4"]
        var major = parseInt(v[0]) || 0
        var minor = parseInt(v[1]) || 0
        return major > 6 || (major === 6 && minor >= 5)
    }
    source: _baseName
        ? (_qt65plus
            ? "qrc:/Logitune/qml/devices/" + _baseName + ".qml"
            : "qrc:/Logitune/qml/" + _baseName + ".qml")
        : ""

    onLoaded: {
        if (!item) return
        if (item.view !== undefined)
            item.view = Qt.binding(function() { return root.view })
        if (item.showHotspots !== undefined)
            item.showHotspots = Qt.binding(function() { return root.showHotspots })
        if (item.buttonClicked !== undefined)
            item.buttonClicked.connect(root.buttonClicked)
    }
}
