"""Đọc/ghi UART trên một QThread riêng để KHÔNG block UI.

Tái dùng state-machine parse khung từ test.py, nhưng đọc theo chunk thay vì
từng byte để giảm overhead. Mọi dữ liệu hợp lệ phát ra qua signal Qt.
"""

from PySide6.QtCore import QThread, Signal, QMutex, QMutexLocker

import serial
import serial.tools.list_ports

from . import protocol


class SerialWorker(QThread):
    telemetryReceived = Signal(dict)   # gói telemetry đã giải mã
    connectionChanged = Signal(bool)   # True khi luồng đọc bắt đầu / False khi đóng
    errorOccurred = Signal(str)
    crcError = Signal()

    _READ_CHUNK = 256

    def __init__(self, parent=None):
        super().__init__(parent)
        self._ser: serial.Serial | None = None
        self._running = False
        self._write_lock = QMutex()

    # ---- tiện ích tĩnh ----
    @staticmethod
    def list_ports() -> list[str]:
        return [p.device for p in serial.tools.list_ports.comports()]

    # ---- vòng đời kết nối ----
    def open(self, port: str, baud: int) -> bool:
        try:
            self._ser = serial.Serial(port, baud, timeout=0.05)
        except Exception as exc:  # noqa: BLE001 - báo lỗi lên UI
            self.errorOccurred.emit(str(exc))
            return False
        self._running = True
        self.start()
        return True

    def close(self):
        self._running = False
        self.wait(1000)
        with QMutexLocker(self._write_lock):
            if self._ser and self._ser.is_open:
                try:
                    self._ser.close()
                except Exception:  # noqa: BLE001
                    pass
            self._ser = None

    def write_frame(self, frame: bytes):
        with QMutexLocker(self._write_lock):
            if self._ser and self._ser.is_open:
                try:
                    self._ser.write(frame)
                except Exception as exc:  # noqa: BLE001
                    self.errorOccurred.emit(str(exc))

    # ---- vòng lặp đọc (chạy trong thread riêng) ----
    def run(self):
        self.connectionChanged.emit(True)
        state = "SOF1"
        pkt_type = 0
        pkt_len = 0
        payload = bytearray()

        try:
            while self._running:
                try:
                    chunk = self._ser.read(self._READ_CHUNK)
                except Exception as exc:  # noqa: BLE001
                    self.errorOccurred.emit(str(exc))
                    break
                if not chunk:
                    continue

                for b in chunk:
                    if state == "SOF1":
                        if b == protocol.SOF1:
                            state = "SOF2"
                    elif state == "SOF2":
                        state = "TYPE" if b == protocol.SOF2 else "SOF1"
                    elif state == "TYPE":
                        pkt_type = b
                        state = "LEN"
                    elif state == "LEN":
                        pkt_len = b
                        payload = bytearray()
                        state = "PAYLOAD" if pkt_len > 0 else "CRC"
                    elif state == "PAYLOAD":
                        payload.append(b)
                        if len(payload) >= pkt_len:
                            state = "CRC"
                    elif state == "CRC":
                        expected = protocol.crc8(bytes([pkt_type, pkt_len]) + payload)
                        if b == expected:
                            if pkt_type == protocol.PKT_TYPE_TELEMETRY:
                                data = protocol.parse_telemetry(bytes(payload))
                                if data is not None:
                                    self.telemetryReceived.emit(data)
                        else:
                            self.crcError.emit()
                        state = "SOF1"
        finally:
            self.connectionChanged.emit(False)
