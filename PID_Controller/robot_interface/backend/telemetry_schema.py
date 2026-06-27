"""Single source of truth cho dữ liệu telemetry.

Muốn thêm/bớt một đường trên đồ thị: chỉ cần thêm/bớt một dòng `Signal(...)`
ở `TELEMETRY_SCHEMA`. Chuỗi unpack, danh sách key, series trên chart và
checkbox legend đều được suy ra TỰ ĐỘNG từ danh sách này.

Lưu ý: thứ tự các Signal phải khớp đúng thứ tự byte trong payload mà STM32 gửi.
"""

from dataclasses import dataclass


@dataclass(frozen=True)
class Signal:
    key: str       # định danh dùng trong code/QML (khớp key của dict telemetry)
    label: str     # nhãn hiển thị trên legend
    fmt: str       # ký tự struct: 'f' (float32) hoặc 'i' (int32)
    motor: str     # 'left' | 'right' — dùng để nhóm hiển thị
    color: str     # màu đường vẽ (hex)
    visible: bool  # mặc định hiện trên đồ thị hay không
    style: str     # "solid" | "dash" — kiểu nét vẽ


# Thứ tự PHẢI khớp payload STM32: <ffffi ffffi>
TELEMETRY_SCHEMA: list[Signal] = [
    Signal("setpoint_left",     "SP L",  "f", "left",  "#2196F3", True,  "dash"),
    Signal("velocity_left",     "Vel L", "f", "left",  "#4CAF50", True,  "solid"),
    Signal("position_left",     "Pos L", "f", "left",  "#9C27B0", False, "solid"),
    Signal("pid_output_left",   "Out L", "f", "left",  "#FF9800", False, "solid"),
    Signal("encoder_raw_left",  "Enc L", "i", "left",  "#795548", False, "solid"),

    Signal("setpoint_right",    "SP R",  "f", "right", "#F44336", True,  "dash"),
    Signal("velocity_right",    "Vel R", "f", "right", "#8BC34A", True,  "solid"),
    Signal("position_right",    "Pos R", "f", "right", "#E91E63", False, "solid"),
    Signal("pid_output_right",  "Out R", "f", "right", "#FFC107", False, "solid"),
    Signal("encoder_raw_right", "Enc R", "i", "right", "#607D8B", False, "solid"),
]

# Suy ra tự động — KHÔNG hardcode ở nơi khác.
TELEMETRY_FORMAT: str = "<" + "".join(s.fmt for s in TELEMETRY_SCHEMA)  # "<ffffiffffi"
TELEMETRY_KEYS: list[str] = [s.key for s in TELEMETRY_SCHEMA]


def schema_for_qml() -> list[dict]:
    """Phơi schema sang QML dưới dạng list các dict (QVariantList)."""
    return [
        {
            "key": s.key,
            "label": s.label,
            "color": s.color,
            "motor": s.motor,
            "visible": s.visible,
            "style": s.style,
        }
        for s in TELEMETRY_SCHEMA
    ]
