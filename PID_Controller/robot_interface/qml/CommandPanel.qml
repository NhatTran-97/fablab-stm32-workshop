import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Frame {
    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        Label {
            text: "Thiết lập PID & Setpoint"
            font.bold: true
            font.pixelSize: 15
        }

        MotorParams {
            id: leftMotor
            title: "Motor Left"
            Layout.fillWidth: true
        }

        MotorParams {
            id: rightMotor
            title: "Motor Right"
            Layout.fillWidth: true
        }

        Button {
            text: "Sao chép Left → Right"
            Layout.fillWidth: true
            onClicked: {
                rightMotor.kp = leftMotor.kp
                rightMotor.ki = leftMotor.ki
                rightMotor.kd = leftMotor.kd
                rightMotor.sp = leftMotor.sp
            }
        }

        Button {
            text: "GỬI XUỐNG STM32"
            highlighted: true
            enabled: controller.connected
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            onClicked: controller.sendCommand(
                leftMotor.kp, leftMotor.ki, leftMotor.kd, leftMotor.sp,
                rightMotor.kp, rightMotor.ki, rightMotor.kd, rightMotor.sp)
        }

        Button {
            text: "Dừng cả 2 (setpoint = 0)"
            Layout.fillWidth: true
            enabled: controller.connected
            onClicked: controller.sendCommand(
                leftMotor.kp, leftMotor.ki, leftMotor.kd, 0,
                rightMotor.kp, rightMotor.ki, rightMotor.kd, 0)
        }

        Item { Layout.fillHeight: true }
    }
}
