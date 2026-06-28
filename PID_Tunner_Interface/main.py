"""Điểm vào ứng dụng: khởi tạo QML engine, nạp Controller & schema."""

import sys
import signal
from pathlib import Path

from PySide6.QtGui import QGuiApplication, QIcon
from PySide6.QtQml import QQmlApplicationEngine
from PySide6.QtQuickControls2 import QQuickStyle

from backend.controller import Controller
from backend.telemetry_schema import schema_for_qml, VIEW_GROUPS


def _base_dir() -> Path:
    """Return base directory — works both as script and PyInstaller bundle."""
    if getattr(sys, 'frozen', False):
        return Path(sys._MEIPASS)
    return Path(__file__).resolve().parent


def main() -> int:
    app = QGuiApplication(sys.argv)
    app.setApplicationName("STM32 PID Tuner")
    QQuickStyle.setStyle("Fusion")

    signal.signal(signal.SIGINT, lambda *_: app.quit())

    icon_path = _base_dir() / "logo" / "logo.png"
    if icon_path.exists():
        app.setWindowIcon(QIcon(str(icon_path)))

    engine = QQmlApplicationEngine()
    ctx = engine.rootContext()
    ctx.setContextProperty("logoSource", icon_path.as_uri() if icon_path.exists() else "")
    controller = Controller()

    app.aboutToQuit.connect(controller.cleanup)

    ctx.setContextProperty("controller", controller)
    ctx.setContextProperty("telemetrySchema", schema_for_qml())
    ctx.setContextProperty("viewGroups", VIEW_GROUPS)

    qml_file = _base_dir() / "qml" / "Main.qml"
    engine.load(qml_file)
    if not engine.rootObjects():
        return -1

    return app.exec()


if __name__ == "__main__":
    sys.exit(main())
