import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Frame {
    RowLayout {
        anchors.fill: parent
        spacing: 10

        Label { text: "Cổng COM:" }
        ComboBox {
            id: portBox
            Layout.preferredWidth: 120
            model: controller.availablePorts
            enabled: !controller.connected
        }
        ToolButton {
            text: "⟳"
            ToolTip.text: "Làm mới danh sách cổng"
            ToolTip.visible: hovered
            enabled: !controller.connected
            onClicked: controller.refreshPorts()
        }

        Label { text: "Baud:" }
        ComboBox {
            id: baudBox
            Layout.preferredWidth: 110
            model: ["9600", "19200", "38400", "57600", "115200", "230400", "460800", "921600"]
            currentIndex: 4   // 115200
            enabled: !controller.connected
        }

        Button {
            text: controller.connected ? "Ngắt kết nối" : "Kết nối"
            highlighted: true
            enabled: controller.connected || portBox.currentText !== ""
            onClicked: {
                if (controller.connected)
                    controller.disconnectSerial()
                else
                    controller.connectSerial(portBox.currentText, parseInt(baudBox.currentText))
            }
        }

        Rectangle {
            width: 14; height: 14; radius: 7
            color: controller.connected ? "#4CAF50" : "#9E9E9E"
        }
        Label { text: controller.connected ? "Online" : "Offline" }

        Item { Layout.fillWidth: true }
    }
}
