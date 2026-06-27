"""Kiểm thử nhanh phần protocol (không cần phần cứng).

Chạy:  python test_protocol.py
"""

import struct

from backend import protocol
from backend.telemetry_schema import TELEMETRY_FORMAT


def test_crc8_matches_reference():
    # so với cách tính bit-by-bit gốc trong test.py
    def ref(data):
        crc = 0
        for byte in data:
            crc ^= byte
            for _ in range(8):
                crc = ((crc << 1) ^ 0x07) & 0xFF if crc & 0x80 else (crc << 1) & 0xFF
        return crc

    for sample in (b"", b"\x01\x02\x03", bytes(range(40))):
        assert protocol.crc8(sample) == ref(sample)


def test_command_frame_roundtrip():
    frame = protocol.build_command_frame(3, 5, 0, 5, 2.5, 4, 0, 8)
    assert frame[0] == protocol.SOF1 and frame[1] == protocol.SOF2
    assert frame[2] == protocol.PKT_TYPE_CMD
    assert frame[3] == protocol.CMD_PAYLOAD_LEN
    assert protocol.crc8(frame[2:-1]) == frame[-1]
    vals = struct.unpack(protocol.CMD_FORMAT, frame[4:-1])
    assert vals == (3, 5, 0, 5, 2.5, 4, 0, 8)


def test_parse_telemetry():
    payload = struct.pack(TELEMETRY_FORMAT, 5.0, 4.8, 1.2, 30.0, 1234,
                                            5.0, 4.9, 1.1, 31.0, 5678)
    data = protocol.parse_telemetry(payload)
    assert data["setpoint_left"] == 5.0
    assert data["encoder_raw_left"] == 1234
    assert data["encoder_raw_right"] == 5678
    assert abs(data["velocity_right"] - 4.9) < 1e-6


def test_parse_telemetry_short_returns_none():
    assert protocol.parse_telemetry(b"\x00" * 10) is None


if __name__ == "__main__":
    for name, fn in list(globals().items()):
        if name.startswith("test_") and callable(fn):
            fn()
            print(f"OK  {name}")
    print("All protocol tests PASSED.")
