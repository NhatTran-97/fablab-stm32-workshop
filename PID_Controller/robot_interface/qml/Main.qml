import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    visible: true
    width: 1280
    height: 800
    title: "STM32 PID Tuner — UART"

    // Hiện thông báo trạng thái từ controller
    footer: Label {
        id: statusBar
        padding: 6
        text: "Sẵn sàng."
        elide: Text.ElideRight
        Connections {
            target: controller
            function onStatusMessage(msg) { statusBar.text = msg }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 8

        ConnectionPanel { Layout.fillWidth: true }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 8

            CommandPanel {
                Layout.preferredWidth: 320
                Layout.fillHeight: true
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 8

                ChartPanel {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                TelemetryPanel {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 110
                }
            }
        }
    }
}
