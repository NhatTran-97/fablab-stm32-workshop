import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root
    spacing: 0

    // nhóm đang hiển thị
    property var activeGroups: ({"velocity": true, "position": false, "encoder": false, "pid_output": false})
    // per-signal visibility
    property var vis: ({})

    Component.onCompleted: {
        let m = {}
        for (let i = 0; i < telemetrySchema.length; i++) {
            let sig = telemetrySchema[i]
            m[sig.key] = sig.visible && root.activeGroups[sig.group]
        }
        vis = m
    }

    function refreshVis() {
        let m = {}
        for (let i = 0; i < telemetrySchema.length; i++) {
            let sig = telemetrySchema[i]
            m[sig.key] = !!root.activeGroups[sig.group]
        }
        vis = m
    }

    Connections {
        target: controller
        function onChartTick() { canvas.requestPaint() }
    }

    // ---- Toolbar: group selector + zoom ----
    Rectangle {
        Layout.fillWidth: true
        height: 38
        color: "#f5f5f5"
        border.color: "#e0e0e0"

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 8; anchors.rightMargin: 8
            spacing: 4

            // Group checkboxes
            Repeater {
                model: viewGroups
                delegate: CheckBox {
                    text: modelData.label
                    checked: modelData.id === "velocity"
                    font.pixelSize: 12
                    palette.windowText: "#333"
                    onToggled: {
                        let g = root.activeGroups
                        g[modelData.id] = checked
                        root.activeGroups = g
                        controller.setGroupActive(modelData.id, checked)
                        root.refreshVis()
                        canvas.requestPaint()
                    }
                }
            }

            Rectangle { width: 1; height: 22; color: "#ddd" }

            Button { text: "X+"; flat: true; implicitWidth: 32; palette.buttonText: "#333"; onClicked: controller.zoomX(0.7) }
            Button { text: "X-"; flat: true; implicitWidth: 32; palette.buttonText: "#333"; onClicked: controller.zoomX(1.43) }
            Button { text: "Y+"; flat: true; implicitWidth: 32; palette.buttonText: "#333"; onClicked: controller.zoomY(0.7) }
            Button { text: "Y-"; flat: true; implicitWidth: 32; palette.buttonText: "#333"; onClicked: controller.zoomY(1.43) }
            Rectangle { width: 1; height: 22; color: "#ddd" }
            Button { text: "Reset"; flat: true; palette.buttonText: "#333"; onClicked: controller.resetView() }
            CheckBox {
                text: "Live"
                checked: controller.follow
                font.pixelSize: 12
                palette.windowText: "#333"
                onToggled: controller.setFollow(checked)
            }
            Item { Layout.fillWidth: true }
        }
    }

    // ---- Chart area ----
    Rectangle {
        Layout.fillWidth: true
        Layout.fillHeight: true
        color: "#1e1e2e"

        Canvas {
            id: canvas
            anchors.fill: parent
            renderStrategy: Canvas.Cooperative

            readonly property int padL: 58
            readonly property int padR: 16
            readonly property int padT: 12
            readonly property int padB: 30

            onPaint: {
                let ctx = getContext("2d")
                ctx.reset()
                let W = width, H = height
                let x0 = padL, x1 = W - padR
                let y0 = padT, y1 = H - padB
                let plotW = x1 - x0, plotH = y1 - y0
                if (plotW <= 0 || plotH <= 0) return

                let xMin = controller.xMin, xMax = controller.xMax
                let yMin = controller.yMin, yMax = controller.yMax
                let xSpan = (xMax - xMin) || 1
                let ySpan = (yMax - yMin) || 1

                function px(x) { return x0 + (x - xMin) / xSpan * plotW }
                function py(y) { return y1 - (y - yMin) / ySpan * plotH }

                ctx.fillStyle = "#252536"
                ctx.fillRect(x0, y0, plotW, plotH)

                // lưới ngang
                ctx.font = "11px 'Segoe UI', Consolas, sans-serif"
                let rows = 6
                for (let r = 0; r <= rows; r++) {
                    let yv = yMin + ySpan * r / rows
                    let yp = Math.round(py(yv)) + 0.5
                    ctx.strokeStyle = "rgba(255,255,255,0.07)"
                    ctx.lineWidth = 1
                    ctx.beginPath(); ctx.moveTo(x0, yp); ctx.lineTo(x1, yp); ctx.stroke()
                    ctx.fillStyle = "#9e9e9e"
                    ctx.textAlign = "right"; ctx.textBaseline = "middle"
                    ctx.fillText(yv.toFixed(1), x0 - 6, yp)
                }

                // lưới dọc
                let step = niceStep(xSpan / 7)
                let tStart = Math.ceil(xMin / step) * step
                ctx.textAlign = "center"; ctx.textBaseline = "top"
                for (let t = tStart; t <= xMax; t += step) {
                    let xp = Math.round(px(t)) + 0.5
                    ctx.strokeStyle = "rgba(255,255,255,0.07)"
                    ctx.lineWidth = 1
                    ctx.beginPath(); ctx.moveTo(xp, y0); ctx.lineTo(xp, y1); ctx.stroke()
                    ctx.fillStyle = "#9e9e9e"
                    ctx.fillText(t.toFixed(step < 1 ? 1 : 0) + "s", xp, y1 + 6)
                }

                // khung
                ctx.strokeStyle = "rgba(255,255,255,0.15)"
                ctx.lineWidth = 1
                ctx.strokeRect(x0 + 0.5, y0 + 0.5, plotW - 1, plotH - 1)

                // clip + vẽ đường
                ctx.save()
                ctx.beginPath(); ctx.rect(x0, y0, plotW, plotH); ctx.clip()
                ctx.lineJoin = "round"
                ctx.lineCap = "round"

                for (let i = 0; i < telemetrySchema.length; i++) {
                    let sig = telemetrySchema[i]
                    if (!root.vis[sig.key]) continue
                    if (!root.activeGroups[sig.group]) continue
                    let pts = controller.pointsFor(sig.key, Math.round(plotW))
                    if (!pts || pts.length < 4) continue

                    ctx.strokeStyle = sig.color
                    if (sig.style === "dash") {
                        ctx.setLineDash([6, 3])
                        ctx.lineWidth = 1.5
                    } else {
                        ctx.setLineDash([])
                        ctx.lineWidth = 1.5
                    }

                    let n = pts.length / 2
                    ctx.beginPath()
                    ctx.moveTo(px(pts[0]), py(pts[1]))

                    if (sig.style === "dash" || n < 3) {
                        for (let k = 2; k < pts.length; k += 2)
                            ctx.lineTo(px(pts[k]), py(pts[k + 1]))
                    } else {
                        let tension = 0.22
                        for (let j = 0; j < n - 1; j++) {
                            let p0x = px(pts[Math.max(j-1, 0)*2]),     p0y = py(pts[Math.max(j-1, 0)*2+1])
                            let p1x = px(pts[j*2]),                     p1y = py(pts[j*2+1])
                            let p2x = px(pts[(j+1)*2]),                 p2y = py(pts[(j+1)*2+1])
                            let p3x = px(pts[Math.min(j+2, n-1)*2]),   p3y = py(pts[Math.min(j+2, n-1)*2+1])
                            let cp1x = p1x + (p2x - p0x) * tension
                            let cp1y = p1y + (p2y - p0y) * tension
                            let cp2x = p2x - (p3x - p1x) * tension
                            let cp2y = p2y - (p3y - p1y) * tension
                            ctx.bezierCurveTo(cp1x, cp1y, cp2x, cp2y, p2x, p2y)
                        }
                    }
                    ctx.stroke()
                }
                ctx.setLineDash([])
                ctx.restore()

                // nhãn trục
                ctx.fillStyle = "#777"
                ctx.font = "10px 'Segoe UI', sans-serif"
                ctx.textAlign = "center"; ctx.textBaseline = "bottom"
                ctx.fillText("Time (s)", x0 + plotW / 2, H - 2)
            }

            function niceStep(raw) {
                if (raw <= 0) return 1
                let p = Math.pow(10, Math.floor(Math.log(raw) / Math.LN10))
                let f = raw / p
                let n = f < 1.5 ? 1 : f < 3 ? 2 : f < 7 ? 5 : 10
                return n * p
            }
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            hoverEnabled: true
            cursorShape: pressed ? Qt.ClosedHandCursor : Qt.OpenHandCursor
            property real lastX: 0
            property real lastY: 0

            onWheel: (wheel) => {
                let f = wheel.angleDelta.y > 0 ? 0.82 : 1.22
                if (wheel.modifiers & Qt.ControlModifier)
                    controller.zoomY(f)
                else
                    controller.zoomX(f)
            }
            onPressed: (mouse) => { lastX = mouse.x; lastY = mouse.y }
            onPositionChanged: (mouse) => {
                if (!pressed) return
                let plotW = canvas.width - canvas.padL - canvas.padR
                let plotH = canvas.height - canvas.padT - canvas.padB
                if (plotW > 0)
                    controller.panX(-(mouse.x - lastX) / plotW * (controller.xMax - controller.xMin))
                if (plotH > 0)
                    controller.panY((mouse.y - lastY) / plotH * (controller.yMax - controller.yMin))
                lastX = mouse.x; lastY = mouse.y
            }
            onDoubleClicked: controller.resetView()
        }
    }

    // ---- Legend (chỉ hiện signal thuộc nhóm đang bật) ----
    Rectangle {
        Layout.fillWidth: true
        implicitHeight: legendFlow.implicitHeight + 10
        color: "#f5f5f5"
        border.color: "#e0e0e0"

        Flow {
            id: legendFlow
            anchors.fill: parent
            anchors.margins: 5
            spacing: 4
            Repeater {
                model: telemetrySchema
                delegate: Rectangle {
                    visible: !!root.activeGroups[modelData.group]
                    width: visible ? legendRow.implicitWidth + 10 : 0
                    height: visible ? 24 : 0
                    radius: 4
                    color: root.vis[modelData.key] ? Qt.rgba(
                        parseInt(modelData.color.substring(1,3),16)/255,
                        parseInt(modelData.color.substring(3,5),16)/255,
                        parseInt(modelData.color.substring(5,7),16)/255,
                        0.15) : "#eeeeee"
                    border.color: root.vis[modelData.key] ? modelData.color : "#cccccc"

                    Row {
                        id: legendRow
                        anchors.centerIn: parent
                        spacing: 4
                        Rectangle {
                            width: 14; height: 3; radius: 1.5
                            color: root.vis[modelData.key] ? modelData.color : "#aaaaaa"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Label {
                            text: modelData.label
                            font.pixelSize: 11
                            font.bold: root.vis[modelData.key]
                            color: root.vis[modelData.key] ? "#333" : "#999"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            let m = root.vis
                            m[modelData.key] = !m[modelData.key]
                            root.vis = m
                            controller.setVisible(modelData.key, m[modelData.key])
                            canvas.requestPaint()
                        }
                    }
                }
            }
        }
    }
}
