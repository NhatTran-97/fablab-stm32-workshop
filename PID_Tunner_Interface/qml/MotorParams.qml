import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

GroupBox {
    id: root
    // Velocity PID
    property real kp: 3.0
    property real ki: 5.0
    property real kd: 0.02
    property real sp: 10.0
    // Position PID
    property real posKp: 7.0
    property real posKi: 0.0
    property real posKd: 0.10

    component NumField : RowLayout {
        property string caption
        property real value: 0
        Label { text: caption; Layout.preferredWidth: 55 }
        TextField {
            Layout.fillWidth: true
            horizontalAlignment: TextInput.AlignRight
            text: value.toFixed(2)
            selectByMouse: true
            inputMethodHints: Qt.ImhFormattedNumbersOnly
            validator: DoubleValidator { decimals: 4; notation: DoubleValidator.StandardNotation }
            onEditingFinished: value = parseFloat(text)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 4

        Label { text: "Velocity PID"; font.bold: true; font.pixelSize: 12 }
        NumField { caption: "Kp"; Layout.fillWidth: true; value: root.kp; onValueChanged: root.kp = value }
        NumField { caption: "Ki"; Layout.fillWidth: true; value: root.ki; onValueChanged: root.ki = value }
        NumField { caption: "Kd"; Layout.fillWidth: true; value: root.kd; onValueChanged: root.kd = value }
        NumField { caption: "SP vel"; Layout.fillWidth: true; value: root.sp; onValueChanged: root.sp = value }

        Rectangle { height: 1; Layout.fillWidth: true; color: "#ccc" }

        Label { text: "Position PID"; font.bold: true; font.pixelSize: 12 }
        NumField { caption: "Kp"; Layout.fillWidth: true; value: root.posKp; onValueChanged: root.posKp = value }
        NumField { caption: "Ki"; Layout.fillWidth: true; value: root.posKi; onValueChanged: root.posKi = value }
        NumField { caption: "Kd"; Layout.fillWidth: true; value: root.posKd; onValueChanged: root.posKd = value }
    }
}
