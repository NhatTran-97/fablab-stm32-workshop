"""Cầu nối QML <-> serial.

- Nhận telemetry, lưu vào buffer cuộn (deque) theo trục thời gian.
- QTimer định kỳ cập nhật trục rồi phát `chartTick` để Canvas vẽ lại.
  Canvas lấy điểm qua `pointsFor()`.
- Hỗ trợ zoom/pan: trục X/Y có thể do người dùng điều khiển (lăn chuột, kéo,
  nút bấm). Chế độ "Live" tự bám dữ liệu mới + auto-scale Y; khi zoom/pan thì
  tạm thoát chế độ tự động cho tới khi Reset.

Không phụ thuộc QtCharts -> tránh access violation của QML ChartView.
"""

import time
from collections import deque

from PySide6.QtCore import QObject, Signal, Slot, Property, QTimer

from .serial_worker import SerialWorker
from . import protocol
from .telemetry_schema import TELEMETRY_KEYS, TELEMETRY_SCHEMA


def _clamp(v, lo, hi):
    return lo if v < lo else hi if v > hi else v


class Controller(QObject):
    connectedChanged = Signal()
    availablePortsChanged = Signal()
    rangeChanged = Signal()
    latestChanged = Signal()
    followChanged = Signal()
    chartTick = Signal()        # báo Canvas vẽ lại
    statusMessage = Signal(str)

    DEFAULT_X_SPAN = 10.0    # cửa sổ thời gian mặc định (s)
    MIN_X_SPAN = 0.5
    MAX_X_SPAN = 600.0
    MAX_POINTS = 8000        # trần điểm/đường (đủ để pan lại lịch sử)
    REDRAW_MS = 33           # ~30 FPS

    def __init__(self, parent=None):
        super().__init__(parent)
        self._worker = SerialWorker(self)
        self._worker.telemetryReceived.connect(self._on_telemetry)
        self._worker.connectionChanged.connect(self._on_connection_changed)
        self._worker.errorOccurred.connect(self.statusMessage)
        self._worker.crcError.connect(self._on_crc_error)

        self._ports: list[str] = []
        self._connected = False
        self._t0 = 0.0
        self._now = 0.0
        self._latest: dict = {}
        self._crc_errors = 0

        self._buffers: dict[str, deque] = {
            k: deque(maxlen=self.MAX_POINTS) for k in TELEMETRY_KEYS
        }
        self._visible: set[str] = {s.key for s in TELEMETRY_SCHEMA if s.visible}

        # trạng thái khung nhìn
        self._x_span = self.DEFAULT_X_SPAN
        self._x_left = 0.0          # mép trái (s) khi không bám realtime
        self._follow = True         # bám dữ liệu mới (Live)
        self._y_auto = True         # tự co giãn trục Y
        self._xmin, self._xmax = 0.0, self.DEFAULT_X_SPAN
        self._ymin, self._ymax = -1.0, 1.0

        self._timer = QTimer(self)
        self._timer.setInterval(self.REDRAW_MS)
        self._timer.timeout.connect(self._redraw)

        self.refreshPorts()

    # ================= Properties cho QML =================
    @Property("QVariantList", notify=availablePortsChanged)
    def availablePorts(self):
        return self._ports

    @Property(bool, notify=connectedChanged)
    def connected(self):
        return self._connected

    @Property(bool, notify=followChanged)
    def follow(self):
        return self._follow


    @Property("QVariantMap", notify=latestChanged)
    def latest(self):
        return self._latest

    @Property(float, notify=rangeChanged)
    def xMin(self):
        return self._xmin

    @Property(float, notify=rangeChanged)
    def xMax(self):
        return self._xmax

    @Property(float, notify=rangeChanged)
    def yMin(self):
        return self._ymin

    @Property(float, notify=rangeChanged)
    def yMax(self):
        return self._ymax

    # ================= Slots: kết nối / lệnh =================
    @Slot()
    def refreshPorts(self):
        self._ports = SerialWorker.list_ports()
        self.availablePortsChanged.emit()

    @Slot(str, int)
    def connectSerial(self, port: str, baud: int):
        if self._connected or not port:
            return
        self.statusMessage.emit(f"Dang mo {port} @ {baud}...")
        self._worker.open(port, baud)

    @Slot()
    def disconnectSerial(self):
        self._worker.close()

    @Slot(float, float, float, float, float, float, float, float)
    def sendCommand(self, kp_l, ki_l, kd_l, sp_l, kp_r, ki_r, kd_r, sp_r):
        if not self._connected:
            self.statusMessage.emit("Chua ket noi UART.")
            return
        frame = protocol.build_command_frame(
            kp_l, ki_l, kd_l, sp_l, kp_r, ki_r, kd_r, sp_r
        )
        self._worker.write_frame(frame)
        self.statusMessage.emit(
            f"[TX] L kp={kp_l} ki={ki_l} kd={kd_l} sp={sp_l} | "
            f"R kp={kp_r} ki={ki_r} kd={kd_r} sp={sp_r}"
        )

    @Slot(str, bool)
    def setVisible(self, key: str, on: bool):
        if on:
            self._visible.add(key)
        else:
            self._visible.discard(key)

    # ================= Slots: zoom / pan =================
    @Slot(float)
    def zoomX(self, factor: float):
        """factor < 1: phóng to (xem ít thời gian hơn); > 1: thu nhỏ."""
        new = _clamp(self._x_span * factor, self.MIN_X_SPAN, self.MAX_X_SPAN)
        if not self._follow:
            center = self._x_left + self._x_span / 2.0
            self._x_left = center - new / 2.0
        self._x_span = new
        self._refresh_view()

    @Slot(float)
    def zoomY(self, factor: float):
        self._y_auto = False
        center = (self._ymin + self._ymax) / 2.0
        half = max((self._ymax - self._ymin) / 2.0 * factor, 1e-3)
        self._ymin, self._ymax = center - half, center + half
        self._refresh_view()

    @Slot(float)
    def panX(self, dx_seconds: float):
        if self._follow:
            self._set_follow(False)
        self._x_left += dx_seconds
        self._refresh_view()

    @Slot(float)
    def panY(self, dy: float):
        self._y_auto = False
        self._ymin += dy
        self._ymax += dy
        self._refresh_view()

    @Slot()
    def resetView(self):
        self._x_span = self.DEFAULT_X_SPAN
        self._y_auto = True
        self._set_follow(True)
        self._refresh_view()

    @Slot(bool)
    def setFollow(self, on: bool):
        self._set_follow(on)
        self._refresh_view()


    @Slot(str, int, result="QVariantList")
    def pointsFor(self, key: str, plot_width: int):
        """Danh sách phẳng [x0,y0,...] trong khung nhìn, với decimation LTTB
        khi số điểm thô > plot_width * 2 (giữ hình dạng đường, bớt gai nhọn).
        plot_width = chiều rộng vùng vẽ (px) để tính ngưỡng decimation."""
        buf = self._buffers.get(key)
        if not buf:
            return []
        lo, hi = self._xmin, self._xmax

        # --- 1. lọc điểm trong khung nhìn + 1 điểm đệm mỗi bên ---
        visible = []
        prev = None
        for x, y in buf:
            if x < lo:
                prev = (x, y)
                continue
            if prev is not None:
                visible.append(prev)
                prev = None
            visible.append((x, y))
            if x > hi:
                break

        if len(visible) < 3:
            return [c for pt in visible for c in pt]

        # --- 2. decimation: LTTB (Largest-Triangle-Three-Buckets) ---
        target = max(plot_width * 2, 60) if plot_width > 0 else 2000
        if len(visible) <= target:
            return [c for pt in visible for c in pt]

        n = len(visible)
        sampled = [visible[0]]
        bucket_size = (n - 2) / (target - 2)
        a_idx = 0
        for i in range(1, target - 1):
            b_start = int((i - 1) * bucket_size) + 1
            b_end = int(i * bucket_size) + 1
            if b_end > n - 1:
                b_end = n - 1

            c_start = int(i * bucket_size) + 1
            c_end = int((i + 1) * bucket_size) + 1
            if c_end > n - 1:
                c_end = n - 1

            # trung bình bucket C
            avg_x = avg_y = 0.0
            c_len = c_end - c_start + 1
            for j in range(c_start, c_end + 1):
                avg_x += visible[j][0]; avg_y += visible[j][1]
            avg_x /= c_len; avg_y /= c_len

            # chọn điểm trong bucket B tạo tam giác lớn nhất
            ax, ay = visible[a_idx]
            best_area = -1.0
            best_idx = b_start
            for j in range(b_start, b_end + 1):
                area = abs((ax - avg_x) * (visible[j][1] - ay)
                           - (ax - visible[j][0]) * (avg_y - ay))
                if area > best_area:
                    best_area = area
                    best_idx = j
            sampled.append(visible[best_idx])
            a_idx = best_idx
        sampled.append(visible[-1])
        return [c for pt in sampled for c in pt]

    @Slot()
    def cleanup(self):
        """Dừng timer + đóng serial sạch trước khi app thoát."""
        self._timer.stop()
        self._worker.close()

    @Slot()
    def clearData(self):
        for buf in self._buffers.values():
            buf.clear()
        self._t0 = time.monotonic()

    # ================= Nội bộ =================
    def _set_follow(self, on: bool):
        if on == self._follow:
            return
        if not on:
            self._x_left = self._xmin   # giữ nguyên vị trí khi rời Live
        self._follow = on
        self.followChanged.emit()

    def _update_x_range(self):
        if self._follow:
            self._xmax = self._now
            self._xmin = self._now - self._x_span
        else:
            self._xmin = self._x_left
            self._xmax = self._x_left + self._x_span

    def _autoscale_y(self):
        ymin, ymax = float("inf"), float("-inf")
        lo, hi = self._xmin, self._xmax
        for key in self._visible:
            for x, y in self._buffers[key]:
                if lo <= x <= hi:
                    if y < ymin:
                        ymin = y
                    if y > ymax:
                        ymax = y
        if ymin <= ymax:
            span = ymax - ymin
            min_span = 5.0
            if span < min_span:
                center = (ymin + ymax) / 2.0
                ymin = center - min_span / 2.0
                ymax = center + min_span / 2.0
            pad = max((ymax - ymin) * 0.08, 0.1)
            self._ymin, self._ymax = ymin - pad, ymax + pad

    def _refresh_view(self):
        """Tính lại trục + yêu cầu vẽ lại ngay (gọi sau zoom/pan)."""
        self._update_x_range()
        if self._y_auto:
            self._autoscale_y()
        self.rangeChanged.emit()
        self.chartTick.emit()

    def _on_connection_changed(self, ok: bool):
        self._connected = ok
        self.connectedChanged.emit()
        if ok:
            self._t0 = time.monotonic()
            self._now = 0.0
            self._crc_errors = 0
            self.clearData()
            self.resetView()
            self._timer.start()
            self.statusMessage.emit("Da ket noi.")
        else:
            self._timer.stop()
            self.statusMessage.emit("Da ngat ket noi.")

    def _on_crc_error(self):
        self._crc_errors += 1
        if self._crc_errors % 20 == 1:
            self.statusMessage.emit(f"CRC error (x{self._crc_errors})")

    def _on_telemetry(self, data: dict):
        t = time.monotonic() - self._t0
        for key, value in data.items():
            buf = self._buffers.get(key)
            if buf is not None:
                buf.append((t, float(value)))
        self._latest = data

    def _redraw(self):
        self._now = (time.monotonic() - self._t0) if self._connected else self._now
        self._update_x_range()
        if self._y_auto:
            self._autoscale_y()
        self.rangeChanged.emit()
        if self._latest:
            self.latestChanged.emit()
        self.chartTick.emit()
