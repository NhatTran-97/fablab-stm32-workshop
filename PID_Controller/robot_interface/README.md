# STM32 PID Tuner (PySide6 + QML)

Giao diện điều chỉnh PID & xem đáp ứng realtime cho robot 2 bánh qua UART.

## Tính năng
- Kết nối UART: chọn cổng COM, baud, Connect/Disconnect (cổng tự liệt kê).
- Gửi Kp/Ki/Kd/Setpoint cho cả 2 motor (Left/Right) xuống STM32.
- Hiển thị số liệu realtime + 1 đồ thị chung; bật/tắt từng đường bằng legend.
- Zoom/pan đồ thị: lăn chuột = zoom X, Ctrl+lăn = zoom Y, kéo = di chuyển,
  double-click = reset; hoặc dùng nút `X± / Y± / Reset` và toggle `Live`.
- **Tự động vẽ theo schema**: mọi signal khai báo trong
  [backend/telemetry_schema.py](backend/telemetry_schema.py) tự xuất hiện trên
  đồ thị, ô số và legend.

## Cài đặt
```bash
pip install -r requirements.txt   # PySide6, pyserial
```

## Chạy
```bash
python main.py
```

## Kiểm thử logic giao thức (không cần phần cứng)
```bash
python test_protocol.py
```

## Cấu trúc
```
main.py                     # nạp QML engine + Controller
backend/
  telemetry_schema.py       # ĐỊNH NGHĨA signal (nguồn của auto-plot)
  protocol.py               # crc8, đóng/giải mã khung
  serial_worker.py          # QThread đọc/ghi UART
  controller.py             # cầu nối QML<->serial, buffer + đẩy điểm chart
qml/
  Main.qml                  # layout tổng
  ConnectionPanel.qml       # kết nối UART
  CommandPanel.qml          # nhập & gửi PID/setpoint
  MotorParams.qml           # nhóm ô nhập 1 motor (tái dùng)
  TelemetryPanel.qml        # số liệu realtime
  ChartPanel.qml            # đồ thị (Canvas thuần QtQuick) + legend toggle
```

> Đồ thị vẽ bằng `Canvas` của QtQuick, **không dùng QtCharts** (QML ChartView
> của PySide6 6.11 gây access violation trên nhiều máy). Canvas ổn định mọi GPU.

## Cách mở rộng
- **Thêm 1 đại lượng vẽ**: thêm một dòng `Signal(...)` vào `TELEMETRY_SCHEMA`
  (đúng thứ tự byte STM32 gửi). Mọi nơi (unpack, chart, legend, ô số) tự cập nhật.
- **Đổi cửa sổ thời gian / FPS**: sửa `WINDOW_SECONDS`, `REDRAW_MS` trong
  [backend/controller.py](backend/controller.py).

## Giao thức UART
Khung: `[0xAA][0x55][type][len][payload...][crc8]` — CRC8 poly `0x07`, init `0x00`,
tính trên `[type, len, payload]`.
- CMD `0x02` (PC→STM32): 8×float32 = `kp,ki,kd,sp` cho left rồi right.
- TELEMETRY `0x01` (STM32→PC): `<ffffi ffffi>` = sp/vel/pos/out/enc cho left & right.

## Lưu ý
Đồ thị dùng `Canvas` thuần QtQuick nên không phụ thuộc driver đồ hoạ của QtCharts.
Tần số vẽ lại mặc định ~30 FPS (`REDRAW_MS` trong controller).
