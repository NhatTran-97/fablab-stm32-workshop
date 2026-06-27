import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Nhóm thông số cho 1 motor. Lộ ra kp/ki/kd/sp (kiểu real) để CommandPanel đọc.
GroupBox {
    id: root
    property real kp: 3.0
    property real ki: 5.0
    property real kd: 0.0
    property real sp: 5.0

    // Ô nhập số thực, đồng bộ hai chiều với một thuộc tính real của root.
    component NumField : RowLayout {
        property string caption
        property real value: 0
        Label { text: caption; Layout.preferredWidth: 70 }
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
        spacing: 6
        NumField { caption: "Kp"; Layout.fillWidth: true; value: root.kp; onValueChanged: root.kp = value }
        NumField { caption: "Ki"; Layout.fillWidth: true; value: root.ki; onValueChanged: root.ki = value }
        NumField { caption: "Kd"; Layout.fillWidth: true; value: root.kd; onValueChanged: root.kd = value }
        NumField { caption: "Setpoint"; Layout.fillWidth: true; value: root.sp; onValueChanged: root.sp = value }
    }
}
