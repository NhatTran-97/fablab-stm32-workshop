"""Kiem thu nhanh phan protocol (khong can phan cung)."""

import struct

from backend import protocol
from backend.telemetry_schema import TELEMETRY_FORMAT


def test_crc8_matches_reference():
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
    frame = protocol.build_command_frame(
        3, 5, 0, 5,       # vel L
        2.5, 4, 0, 8,     # vel R
        2.0, 0, 0,        # pos L
        2.0, 0, 0,        # pos R
    )
    assert frame[0] == protocol.SOF1 and frame[1] == protocol.SOF2
    assert frame[2] == protocol.PKT_TYPE_CMD
    assert frame[3] == protocol.CMD_PAYLOAD_LEN  # 56
    assert protocol.crc8(frame[2:-1]) == frame[-1]
    vals = struct.unpack(protocol.CMD_FORMAT, frame[4:-1])
    assert vals == (3, 5, 0, 5, 2.5, 4, 0, 8, 2.0, 0, 0, 2.0, 0, 0)


def test_parse_telemetry():
    payload = struct.pack(TELEMETRY_FORMAT,
                          5.0, 4.8, 1.2, 30.0, 1234,     # left
                          5.0, 4.9, 1.1, 31.0, 5678,     # right
                          0,                               # mode (VEL)
                          0.0, 0.0)                        # pos sp L, R
    data = protocol.parse_telemetry(payload)
    assert data["setpoint_left"] == 5.0
    assert data["encoder_raw_left"] == 1234
    assert data["encoder_raw_right"] == 5678
    assert abs(data["velocity_right"] - 4.9) < 1e-6
    assert data["mode"] == 0
    assert data["pos_setpoint_left"] == 0.0
    assert data["pos_setpoint_right"] == 0.0


def test_parse_telemetry_short_returns_none():
    assert protocol.parse_telemetry(b"\x00" * 10) is None


if __name__ == "__main__":
    for name, fn in list(globals().items()):
        if name.startswith("test_") and callable(fn):
            fn()
            print(f"OK  {name}")
    print("All protocol tests PASSED.")
