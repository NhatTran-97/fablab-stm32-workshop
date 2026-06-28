"""Cầu nối QML <-> serial.

Hỗ trợ zoom/pan, auto-scale Y theo nhóm đang hiển thị (velocity/position/encoder/pid_output).
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
    chartTick = Signal()
    statusMessage = Signal(str)

    DEFAULT_X_SPAN = 10.0
    MIN_X_SPAN = 0.5
    MAX_X_SPAN = 600.0
    MAX_POINTS = 8000
    REDRAW_MS = 33

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
        # signal -> group lookup
        self._sig_group: dict[str, str] = {s.key: s.group for s in TELEMETRY_SCHEMA}
        # nhóm đang hiển thị (dùng cho auto-scale Y)
        self._active_groups: set[str] = {"velocity"}
        # per-signal visibility (trong nhóm đang bật)
        self._visible: set[str] = set()
        self._update_visible_from_groups()

        self._x_span = self.DEFAULT_X_SPAN
        self._x_left = 0.0
        self._follow = True
        self._y_auto = True
        self._xmin, self._xmax = 0.0, self.DEFAULT_X_SPAN
        self._ymin, self._ymax = -1.0, 1.0

        self._timer = QTimer(self)
        self._timer.setInterval(self.REDRAW_MS)
        self._timer.timeout.connect(self._redraw)

        self.refreshPorts()

    def _update_visible_from_groups(self):
        self._visible = {
            s.key for s in TELEMETRY_SCHEMA
            if s.group in self._active_groups and s.visible
        }

    # ================= Properties =================
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

    @Slot("QVariantList")
    def sendCommand(self, params):
        """Nhận list 14 float từ QML."""
        if not self._connected:
            self.statusMessage.emit("Chua ket noi UART.")
            return
        if len(params) < 14:
            self.statusMessage.emit("Thieu tham so.")
            return
        p = [float(x) for x in params]
        frame = protocol.build_command_frame(*p)
        self._worker.write_frame(frame)
        self.statusMessage.emit(
            f"[TX] Vel L kp={p[0]} ki={p[1]} kd={p[2]} sp={p[3]} | "
            f"Vel R kp={p[4]} ki={p[5]} kd={p[6]} sp={p[7]} | "
            f"Pos L kp={p[8]} ki={p[9]} kd={p[10]} | "
            f"Pos R kp={p[11]} ki={p[12]} kd={p[13]}"
        )

    @Slot(str, bool)
    def setGroupActive(self, group: str, on: bool):
        if on:
            self._active_groups.add(group)
        else:
            self._active_groups.discard(group)
        self._update_visible_from_groups()
        self._y_auto = True
        self._refresh_view()

    @Slot(str, bool)
    def setVisible(self, key: str, on: bool):
        if on:
            self._visible.add(key)
        else:
            self._visible.discard(key)

    # ================= Slots: zoom / pan =================
    @Slot(float)
    def zoomX(self, factor: float):
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
        buf = self._buffers.get(key)
        if not buf:
            return []
        lo, hi = self._xmin, self._xmax

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

        target = max(plot_width * 2, 60) if plot_width > 0 else 2000
        if len(visible) <= target:
            return [c for pt in visible for c in pt]

        n = len(visible)
        sampled = [visible[0]]
        bucket_size = (n - 2) / (target - 2)
        a_idx = 0
        for i in range(1, target - 1):
            b_start = int((i - 1) * bucket_size) + 1
            b_end = min(int(i * bucket_size) + 1, n - 1)
            c_start = int(i * bucket_size) + 1
            c_end = min(int((i + 1) * bucket_size) + 1, n - 1)

            avg_x = avg_y = 0.0
            c_len = c_end - c_start + 1
            for j in range(c_start, c_end + 1):
                avg_x += visible[j][0]; avg_y += visible[j][1]
            avg_x /= c_len; avg_y /= c_len

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
            self._x_left = self._xmin
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
