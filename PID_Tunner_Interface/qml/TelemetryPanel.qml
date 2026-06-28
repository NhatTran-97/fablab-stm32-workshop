import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Frame {
    ColumnLayout {
        anchors.fill: parent
        spacing: 6

        Label { text: "Du lieu realtime"; font.bold: true }

        GridLayout {
            columns: 4
            columnSpacing: 16
            rowSpacing: 4
            Layout.fillWidth: true

            Repeater {
                model: telemetrySchema
                delegate: RowLayout {
                    spacing: 6
                    Rectangle { width: 10; height: 10; radius: 5; color: modelData.color }
                    Label { text: modelData.label + ":"; Layout.preferredWidth: 65 }
                    Label {
                        property var v: controller.latest[modelData.key]
                        text: v === undefined ? "--" : Number(v).toFixed(2)
                        font.family: "Consolas, monospace"
                        font.bold: true
                    }
                }
            }

            // Mode indicator
            RowLayout {
                spacing: 6
                Rectangle { width: 10; height: 10; radius: 5; color: "#FFFFFF" }
                Label { text: "Mode:"; Layout.preferredWidth: 65 }
                Label {
                    property var m: controller.latest["mode"]
                    text: m === undefined ? "--" : (m ? "POS" : "VEL")
                    font.family: "Consolas, monospace"
                    font.bold: true
                    color: m ? "#FF9800" : "#4CAF50"
                }
            }
        }
    }
}
