"""Single source of truth cho dữ liệu telemetry.

Thứ tự Signal khớp thứ tự byte trong payload STM32 gửi: <ffffiffffiBff>
Mỗi signal thuộc 1 nhóm (group) để UI cho người dùng chọn xem theo nhóm.
"""

from dataclasses import dataclass


@dataclass(frozen=True)
class Signal:
    key: str
    label: str
    fmt: str        # 'f' (float32), 'i' (int32), 'B' (uint8)
    motor: str      # 'left' | 'right' | 'system'
    color: str
    visible: bool   # mặc định hiện khi nhóm được chọn
    style: str      # "solid" | "dash"
    group: str      # nhóm hiển thị: "velocity" | "position" | "encoder" | "pid_output"


# Thứ tự PHẢI khớp payload STM32: <ffffi ffffi B ff>
TELEMETRY_SCHEMA: list[Signal] = [
    # -- Left motor --
    Signal("setpoint_left",      "SP Vel L",  "f", "left",  "#2196F3", True,  "dash",  "velocity"),
    Signal("velocity_left",      "Vel L",     "f", "left",  "#4CAF50", True,  "solid", "velocity"),
    Signal("position_left",      "Pos L",     "f", "left",  "#9C27B0", True,  "solid", "position"),
    Signal("pid_output_left",    "Out L",     "f", "left",  "#FF9800", True,  "solid", "pid_output"),
    Signal("encoder_raw_left",   "Enc L",     "i", "left",  "#795548", True,  "solid", "encoder"),
    # -- Right motor --
    Signal("setpoint_right",     "SP Vel R",  "f", "right", "#F44336", True,  "dash",  "velocity"),
    Signal("velocity_right",     "Vel R",     "f", "right", "#8BC34A", True,  "solid", "velocity"),
    Signal("position_right",     "Pos R",     "f", "right", "#E91E63", True,  "solid", "position"),
    Signal("pid_output_right",   "Out R",     "f", "right", "#FFC107", True,  "solid", "pid_output"),
    Signal("encoder_raw_right",  "Enc R",     "i", "right", "#607D8B", True,  "solid", "encoder"),
    # -- System --
    Signal("mode",               "Mode",      "B", "system","#FFFFFF", False, "solid", ""),
    # -- Position setpoints --
    Signal("pos_setpoint_left",  "SP Pos L",  "f", "left",  "#03A9F4", True,  "dash",  "position"),
    Signal("pos_setpoint_right", "SP Pos R",  "f", "right", "#FF5722", True,  "dash",  "position"),
]

TELEMETRY_FORMAT: str = "<" + "".join(s.fmt for s in TELEMETRY_SCHEMA)
TELEMETRY_KEYS: list[str] = [s.key for s in TELEMETRY_SCHEMA]

# Nhóm hiển thị cho QML
VIEW_GROUPS = [
    {"id": "velocity",   "label": "Toc do",      "icon": "V"},
    {"id": "position",   "label": "Vi tri",      "icon": "P"},
    {"id": "encoder",    "label": "Encoder",     "icon": "E"},
    {"id": "pid_output", "label": "PID Output",  "icon": "O"},
]


def schema_for_qml() -> list[dict]:
    return [
        {
            "key": s.key,
            "label": s.label,
            "color": s.color,
            "motor": s.motor,
            "visible": s.visible,
            "style": s.style,
            "group": s.group,
        }
        for s in TELEMETRY_SCHEMA
        if s.group  # bỏ qua 'mode' (không vẽ)
    ]
