import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Frame {
    ColumnLayout {
        anchors.fill: parent
        spacing: 6

        Label { text: "Dữ liệu realtime"; font.bold: true }

        GridLayout {
            columns: 4
            columnSpacing: 16
            rowSpacing: 4
            Layout.fillWidth: true

            // Sinh ô số tự động theo schema -> thêm signal là tự xuất hiện ở đây.
            Repeater {
                model: telemetrySchema
                delegate: RowLayout {
                    spacing: 6
                    Rectangle { width: 10; height: 10; radius: 5; color: modelData.color }
                    Label { text: modelData.label + ":"; Layout.preferredWidth: 50 }
                    Label {
                        property var v: controller.latest[modelData.key]
                        text: v === undefined ? "—" : Number(v).toFixed(2)
                        font.family: "Consolas, monospace"
                        font.bold: true
                    }
                }
            }
        }
    }
}
