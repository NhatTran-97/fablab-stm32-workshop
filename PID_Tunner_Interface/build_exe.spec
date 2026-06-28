# -*- mode: python ; coding: utf-8 -*-
"""PyInstaller spec for STM32 PID Tuner."""

import os
from PyInstaller.utils.hooks import collect_submodules

block_cipher = None
ROOT = os.path.abspath('.')
QML_DIR = os.path.join(ROOT, 'qml')

# Our QML files
qml_datas = [(os.path.join(QML_DIR, f), 'qml')
             for f in os.listdir(QML_DIR) if f.endswith('.qml')]

# Logo
LOGO_DIR = os.path.join(ROOT, 'logo')
if os.path.isdir(LOGO_DIR):
    for f in os.listdir(LOGO_DIR):
        if f.lower().endswith(('.png', '.ico')):
            qml_datas.append((os.path.join(LOGO_DIR, f), 'logo'))

hidden = ['PySide6.QtGui', 'PySide6.QtCore', 'PySide6.QtNetwork',
          'PySide6.QtQuick', 'PySide6.QtQml', 'PySide6.QtQuickControls2',
          'PySide6.QtOpenGL',
          'serial', 'serial.tools', 'serial.tools.list_ports']

EXCLUDES = [
    'tkinter', 'matplotlib', 'numpy', 'scipy', 'PIL',
    'PySide6.QtWebEngine', 'PySide6.QtWebEngineCore', 'PySide6.QtWebEngineWidgets',
    'PySide6.Qt3DCore', 'PySide6.Qt3DRender', 'PySide6.Qt3DInput',
    'PySide6.Qt3DLogic', 'PySide6.Qt3DExtras', 'PySide6.Qt3DAnimation',
    'PySide6.QtQuick3D',
    'PySide6.QtMultimedia', 'PySide6.QtMultimediaWidgets',
    'PySide6.QtBluetooth', 'PySide6.QtNfc',
    'PySide6.QtPositioning', 'PySide6.QtLocation',
    'PySide6.QtSensors', 'PySide6.QtSerialBus',
    'PySide6.QtCharts', 'PySide6.QtDataVisualization',
    'PySide6.QtRemoteObjects', 'PySide6.QtScxml',
    'PySide6.QtVirtualKeyboard', 'PySide6.QtPdf', 'PySide6.QtPdfWidgets',
    'PySide6.QtSpatialAudio', 'PySide6.QtHttpServer',
    'PySide6.QtDesigner', 'PySide6.QtHelp', 'PySide6.QtTest',
    'PySide6.QtWidgets', 'PySide6.QtSvg', 'PySide6.QtSvgWidgets',
    'PySide6.QtDBus', 'PySide6.QtXml',
]

a = Analysis(
    ['main.py'],
    pathex=[ROOT],
    binaries=[],
    datas=qml_datas,
    hiddenimports=hidden,
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=EXCLUDES,
    cipher=block_cipher,
    noarchive=False,
)

# Remove Qt3D / Quick3D / Multimedia binaries that sneak in via hooks
STRIP_PREFIXES = ('Qt6Quick3D', 'Qt63D', 'Qt6Multimedia', 'Qt6Spatial',
                  'Qt6Pdf', 'Qt6WebEngine', 'Qt6Charts', 'Qt6DataVis',
                  'Qt6VirtualKeyboard', 'Qt6Bluetooth', 'Qt6Nfc',
                  'Qt6Designer', 'Qt6Help', 'Qt6Svg', 'Qt6Widgets',
                  'Qt6RemoteObjects', 'Qt6Scxml', 'Qt6SerialBus',
                  'Qt6HttpServer', 'Qt6Location', 'Qt6Positioning',
                  'Qt6Sensors', 'Qt6Test')
a.binaries = [b for b in a.binaries
              if not any(os.path.basename(b[0]).startswith(p) for p in STRIP_PREFIXES)]
a.datas = [d for d in a.datas
           if not any(x in d[0] for x in ('QtQuick3D', 'Qt3D', 'QtMultimedia',
                                           'QtWebEngine', 'QtCharts', 'QtDataVisualization',
                                           'QtVirtualKeyboard', 'QtBluetooth',
                                           'QtDesigner', 'MaterialEditor',
                                           'QtSvg', 'QtWidgets', 'QtPdf'))]

pyz = PYZ(a.pure, a.zipped_data, cipher=block_cipher)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.datas,
    [],
    name='STM32_PID_Tuner',
    debug=False,
    bootloader_ignore_signals=False,
    strip=False,
    upx=True,
    console=False,
    icon=os.path.join(ROOT, 'logo.ico'),
)
