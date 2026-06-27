"""Điểm vào ứng dụng: khởi tạo QML engine, nạp Controller & schema."""

import sys
import signal
from pathlib import Path

from PySide6.QtGui import QGuiApplication
from PySide6.QtQml import QQmlApplicationEngine
from PySide6.QtQuickControls2 import QQuickStyle

from backend.controller import Controller
from backend.telemetry_schema import schema_for_qml


def main() -> int:
    app = QGuiApplication(sys.argv)
    app.setApplicationName("STM32 PID Tuner")
    QQuickStyle.setStyle("Fusion")

    # Ctrl+C trong terminal -> thoát sạch qua Qt event loop (không traceback)
    signal.signal(signal.SIGINT, lambda *_: app.quit())

    engine = QQmlApplicationEngine()
    controller = Controller()

    app.aboutToQuit.connect(controller.cleanup)

    ctx = engine.rootContext()
    ctx.setContextProperty("controller", controller)
    ctx.setContextProperty("telemetrySchema", schema_for_qml())

    qml_file = Path(__file__).resolve().parent / "qml" / "Main.qml"
    engine.load(qml_file)
    if not engine.rootObjects():
        return -1

    return app.exec()


if __name__ == "__main__":
    sys.exit(main())
