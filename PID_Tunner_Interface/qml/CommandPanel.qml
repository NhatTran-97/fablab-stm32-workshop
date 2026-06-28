import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Frame {
    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Label {
            text: "Thiet lap PID"
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
            text: "Copy Left -> Right"
            Layout.fillWidth: true
            onClicked: {
                rightMotor.kp = leftMotor.kp
                rightMotor.ki = leftMotor.ki
                rightMotor.kd = leftMotor.kd
                rightMotor.sp = leftMotor.sp
                rightMotor.posKp = leftMotor.posKp
                rightMotor.posKi = leftMotor.posKi
                rightMotor.posKd = leftMotor.posKd
            }
        }

        Button {
            text: "GUI XUONG STM32"
            highlighted: true
            enabled: controller.connected
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            onClicked: controller.sendCommand([
                leftMotor.kp, leftMotor.ki, leftMotor.kd, leftMotor.sp,
                rightMotor.kp, rightMotor.ki, rightMotor.kd, rightMotor.sp,
                leftMotor.posKp, leftMotor.posKi, leftMotor.posKd,
                rightMotor.posKp, rightMotor.posKi, rightMotor.posKd
            ])
        }

        Button {
            text: "Dung ca 2 (SP vel = 0)"
            Layout.fillWidth: true
            enabled: controller.connected
            onClicked: controller.sendCommand([
                leftMotor.kp, leftMotor.ki, leftMotor.kd, 0,
                rightMotor.kp, rightMotor.ki, rightMotor.kd, 0,
                leftMotor.posKp, leftMotor.posKi, leftMotor.posKd,
                rightMotor.posKp, rightMotor.posKi, rightMotor.posKd
            ])
        }

        Item { Layout.fillHeight: true }
    }
}
