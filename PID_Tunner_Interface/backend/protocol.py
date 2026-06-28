"""Logic giao thức thuần (không phụ thuộc serial/Qt).

Khung truyền:  [SOF1][SOF2][type][len][payload...][crc8]
  - CMD 0x02 (PC -> STM32): 14 float32 = vel kp/ki/kd/sp L&R + pos kp/ki/kd L&R
  - TELEMETRY 0x01 (STM32 -> PC): <ffffiffffiBff> (49 bytes)
CRC8: poly 0x07, init 0x00, tính trên [type, len, payload...].
"""

import struct

from .telemetry_schema import TELEMETRY_FORMAT, TELEMETRY_KEYS

SOF1 = 0xAA
SOF2 = 0x55
PKT_TYPE_TELEMETRY = 0x01
PKT_TYPE_CMD = 0x02

CMD_FORMAT = "<ffffffffffffff"       # 14 floats = 56 bytes
CMD_PAYLOAD_LEN = struct.calcsize(CMD_FORMAT)
TELEMETRY_PAYLOAD_LEN = struct.calcsize(TELEMETRY_FORMAT)

_CRC8_TABLE = []
for _b in range(256):
    _c = _b
    for _ in range(8):
        _c = ((_c << 1) ^ 0x07) & 0xFF if (_c & 0x80) else (_c << 1) & 0xFF
    _CRC8_TABLE.append(_c)


def crc8(data: bytes) -> int:
    crc = 0x00
    for byte in data:
        crc = _CRC8_TABLE[crc ^ byte]
    return crc


def build_command_frame(
    kp_l, ki_l, kd_l, sp_l,
    kp_r, ki_r, kd_r, sp_r,
    pos_kp_l, pos_ki_l, pos_kd_l,
    pos_kp_r, pos_ki_r, pos_kd_r,
) -> bytes:
    payload = struct.pack(
        CMD_FORMAT,
        kp_l, ki_l, kd_l, sp_l,
        kp_r, ki_r, kd_r, sp_r,
        pos_kp_l, pos_ki_l, pos_kd_l,
        pos_kp_r, pos_ki_r, pos_kd_r,
    )
    header = bytes([PKT_TYPE_CMD, len(payload)])
    crc = crc8(header + payload)
    return bytes([SOF1, SOF2]) + header + payload + bytes([crc])


def parse_telemetry(payload: bytes) -> dict | None:
    if len(payload) < TELEMETRY_PAYLOAD_LEN:
        return None
    values = struct.unpack(TELEMETRY_FORMAT, payload[:TELEMETRY_PAYLOAD_LEN])
    return dict(zip(TELEMETRY_KEYS, values))
