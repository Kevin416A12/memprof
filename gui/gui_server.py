    #!/usr/bin/env python3
# GUI completa con 4 pestañas y gráficas para el Memory Profiler.
import sys, socket, threading, json, queue, time
from collections import defaultdict, deque
from dataclasses import dataclass

from    PySide6 import QtWidgets, QtCore



from PySide6.QtWidgets import (
    QLabel, QVBoxLayout, QWidget, QHBoxLayout, QTabWidget, QTableWidget,
    QTableWidgetItem, QHeaderView, QGroupBox, QGridLayout, QAbstractItemView
)

# Matplotlib embebido
from matplotlib.backends.backend_qtagg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure

HOST = "0.0.0.0"
PORT = 5555

# ----------- Modelo de datos en memoria -----------
@dataclass
class Alloc:
    addr: str
    size: int
    file: str
    line: int
    type_name: str
    is_array: bool
    t_ms: int

class ProfilerModel:
    def __init__(self):
        self.live = {}  # addr -> Alloc
        self.curr_bytes = 0
        self.max_bytes = 0
        self.total_allocs = 0
        self.total_frees = 0

        self.timeline_t = deque(maxlen=5000)     # timestamps
        self.timeline_b = deque(maxlen=5000)     # bytes

        self.by_file_bytes = defaultdict(int)    # file -> total bytes allocados
        self.by_file_count = defaultdict(int)    # file -> cantidad de allocs

        self.leak_summary = {"liveCount": 0, "leakBytes": 0, "time": 0}
        self.last_event_ms = 0

    def on_alloc(self, j):
        addr = j.get("addr","")
        a = Alloc(
            addr=addr,
            size=int(j.get("size",0)),
            file=j.get("file","?"),
            line=int(j.get("line",0)),
            type_name=j.get("typeName","unknown"),
            is_array=("type" in j and j["type"]=="ALLOC" and False) or False,
            t_ms=int(j.get("t",0))
        )
        self.live[addr] = a
        self.by_file_bytes[a.file] += a.size
        self.by_file_count[a.file] += 1
        self.total_allocs += 1
        self.last_event_ms = a.t_ms

    def on_free(self, j):
        addr = j.get("addr","")
        if addr in self.live:
            a = self.live.pop(addr)
            self.total_frees += 1
        self.last_event_ms = int(j.get("t",0))

    def on_snapshot(self, j):
        self.curr_bytes = int(j.get("currBytes", self.curr_bytes))
        self.max_bytes = max(self.max_bytes, int(j.get("maxBytes", 0)))
        live_count = int(j.get("liveCount", len(self.live)))  # no lo usamos directo
        t = int(j.get("t", 0))
        self.timeline_t.append(t)
        self.timeline_b.append(self.curr_bytes)
        self.last_event_ms = t

    def on_leak_summary(self, j):
        self.leak_summary = {
            "liveCount": int(j.get("liveCount",0)),
            "leakBytes": int(j.get("leakBytes",0)),
            "time": int(j.get("t",0)),
        }
        self.last_event_ms = self.leak_summary["time"]

    def top3_files(self):
        # Top por bytes
        items = sorted(self.by_file_bytes.items(), key=lambda kv: kv[1], reverse=True)
        return items[:3]

# ----------- Thread servidor TCP -----------
class ReceiverThread(threading.Thread):
    def __init__(self, q):
        super().__init__(daemon=True)
        self.q = q

    def run(self):
        srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        srv.bind((HOST, PORT))
        srv.listen(1)
        print(f"[GUI] listening on {HOST}:{PORT}")
        while True:
            conn, addr = srv.accept()
            print("[GUI] connected by", addr)
            with conn:
                buf = b""
                while True:
                    data = conn.recv(4096)
                    if not data:
                        break
                    buf += data
                    while b"\n" in buf:
                        line, buf = buf.split(b"\n", 1)
                        try:
                            j = json.loads(line.decode("utf-8"))
                            self.q.put(j)
                        except Exception as e:
                            print("[GUI] json parse error:", e)

# ----------- Widgets auxiliares -----------
class MplFigure(QWidget):
    def __init__(self, title=""):
        super().__init__()
        self.fig = Figure(figsize=(5,3), tight_layout=True)
        self.canvas = FigureCanvas(self.fig)
        self.ax = self.fig.add_subplot(111)
        if title:
            self.ax.set_title(title)
        layout = QVBoxLayout()
        layout.addWidget(self.canvas)
        self.setLayout(layout)

    def plot_line(self, x, y, xlabel="", ylabel=""):
        self.ax.clear()
        self.ax.plot(x, y)
        self.ax.set_xlabel(xlabel)
        self.ax.set_ylabel(ylabel)
        self.canvas.draw_idle()

    def bar(self, labels, values, xlabel="", ylabel=""):
        self.ax.clear()
        self.ax.bar(labels, values)
        self.ax.set_xlabel(xlabel)
        self.ax.set_ylabel(ylabel)
        self.ax.tick_params(axis='x', rotation=30)
        self.canvas.draw_idle()

    def pie(self, labels, values):
        self.ax.clear()
        if sum(values) > 0:
            self.ax.pie(values, labels=labels, autopct="%1.1f%%")
        self.canvas.draw_idle()

# ----------- Tabs -----------
class TabOverview(QWidget):
    def __init__(self, model: ProfilerModel):
        super().__init__()
        self.model = model

        # KPIs
        self.lbl_curr = QLabel("0 MB")
        self.lbl_live = QLabel("0")
        self.lbl_max  = QLabel("0 MB")
        self.lbl_total = QLabel("0")
        self.lbl_leaks = QLabel("0 MB")

        kpi = QGridLayout()
        kpi.addWidget(QLabel("Uso actual (MB):"), 0,0); kpi.addWidget(self.lbl_curr, 0,1)
        kpi.addWidget(QLabel("Asignaciones activas:"), 1,0); kpi.addWidget(self.lbl_live, 1,1)
        kpi.addWidget(QLabel("Uso máximo (MB):"), 2,0); kpi.addWidget(self.lbl_max, 2,1)
        kpi.addWidget(QLabel("Total asignaciones:"), 3,0); kpi.addWidget(self.lbl_total, 3,1)
        kpi.addWidget(QLabel("Leaks (MB):"), 4,0); kpi.addWidget(self.lbl_leaks, 4,1)
        box_kpi = QGroupBox("Métricas generales")
        box_kpi.setLayout(kpi)

        # Timeline
        self.timeline = MplFigure("Línea de tiempo (MB)")

        # Top 3
        self.tbl_top = QTableWidget(0, 3)
        self.tbl_top.setHorizontalHeaderLabels(["Archivo", "Asignaciones", "MB"])
        self.tbl_top.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        self.tbl_top.setEditTriggers(QAbstractItemView.NoEditTriggers)
        box_top = QGroupBox("Top 3 archivos por memoria asignada")
        v = QVBoxLayout(); v.addWidget(self.tbl_top); box_top.setLayout(v)

        layout = QVBoxLayout()
        layout.addWidget(box_kpi)
        layout.addWidget(self.timeline)
        layout.addWidget(box_top)
        self.setLayout(layout)

    @staticmethod
    def _mb(bytes_): return round(bytes_ / (1024*1024), 3)

    def refresh(self):
        # KPIs
        self.lbl_curr.setText(str(self._mb(self.model.curr_bytes)))
        self.lbl_max.setText(str(self._mb(self.model.max_bytes)))
        self.lbl_total.setText(str(self.model.total_allocs))
        self.lbl_live.setText(str(len(self.model.live)))
        self.lbl_leaks.setText(str(self._mb(self.model.leak_summary.get("leakBytes",0))))

        # Timeline
        # Convertimos t_ms -> segundos relativos
        if self.model.timeline_t:
            t0 = self.model.timeline_t[0] / 1000.0
            xs = [(t/1000.0 - t0) for t in self.model.timeline_t]
            ys = [self._mb(b) for b in self.model.timeline_b]
            self.timeline.plot_line(xs, ys, "tiempo (s)", "MB")

        # Top 3
        top = self.model.top3_files()
        self.tbl_top.setRowCount(len(top))
        for r, (fname, bytes_) in enumerate(top):
            count = self.model.by_file_count.get(fname, 0)
            self.tbl_top.setItem(r, 0, QTableWidgetItem(fname))
            self.tbl_top.setItem(r, 1, QTableWidgetItem(str(count)))
            self.tbl_top.setItem(r, 2, QTableWidgetItem(str(self._mb(bytes_))))

class TabMemMap(QWidget):
    def __init__(self, model: ProfilerModel):
        super().__init__()
        self.model = model
        self.tbl = QTableWidget(0, 6)
        self.tbl.setHorizontalHeaderLabels(["Dirección", "MB", "Tipo", "Archivo", "Línea", "Array?"])
        self.tbl.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        self.tbl.setEditTriggers(QAbstractItemView.NoEditTriggers)
        layout = QVBoxLayout(); layout.addWidget(self.tbl); self.setLayout(layout)

    @staticmethod
    def _mb(bytes_): return round(bytes_ / (1024*1024), 6)

    def refresh(self):
        live = list(self.model.live.values())
        self.tbl.setRowCount(len(live))
        for r, a in enumerate(live):
            self.tbl.setItem(r, 0, QTableWidgetItem(a.addr))
            self.tbl.setItem(r, 1, QTableWidgetItem(str(self._mb(a.size))))
            self.tbl.setItem(r, 2, QTableWidgetItem(a.type_name))
            self.tbl.setItem(r, 3, QTableWidgetItem(a.file))
            self.tbl.setItem(r, 4, QTableWidgetItem(str(a.line)))
            self.tbl.setItem(r, 5, QTableWidgetItem("sí" if a.is_array else "no"))

class TabByFile(QWidget):
    def __init__(self, model: ProfilerModel):
        super().__init__()
        self.model = model
        self.tbl = QTableWidget(0, 3)
        self.tbl.setHorizontalHeaderLabels(["Archivo", "Asignaciones", "MB"])
        self.tbl.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        self.tbl.setEditTriggers(QAbstractItemView.NoEditTriggers)

        self.bar = MplFigure("Asignación por archivo (MB)")

        lay = QVBoxLayout()
        lay.addWidget(self.tbl)
        lay.addWidget(self.bar)
        self.setLayout(lay)

    @staticmethod
    def _mb(bytes_): return round(bytes_ / (1024*1024), 3)

    def refresh(self):
        items = sorted(self.model.by_file_bytes.items(), key=lambda kv: kv[1], reverse=True)
        self.tbl.setRowCount(len(items))
        labels = []; values = []
        for r, (fname, bytes_) in enumerate(items):
            count = self.model.by_file_count.get(fname, 0)
            labels.append(fname if len(fname) < 30 else "..." + fname[-27:])
            values.append(self._mb(bytes_))
            self.tbl.setItem(r, 0, QTableWidgetItem(fname))
            self.tbl.setItem(r, 1, QTableWidgetItem(str(count)))
            self.tbl.setItem(r, 2, QTableWidgetItem(str(self._mb(bytes_))))
        # barras (si hay muchos, muestra top 10)
        show = min(10, len(labels))
        self.bar.bar(labels[:show], values[:show], "archivo", "MB")

class TabLeaks(QWidget):
    def __init__(self, model: ProfilerModel):
        super().__init__()
        self.model = model
        # KPIs
        self.lbl_total = QLabel("0 MB")
        self.lbl_biggest = QLabel("0 MB")
        self.lbl_file_most = QLabel("-")
        self.lbl_rate = QLabel("0.0 %")

        kpi = QGridLayout()
        kpi.addWidget(QLabel("Total fugado (MB):"), 0,0); kpi.addWidget(self.lbl_total, 0,1)
        kpi.addWidget(QLabel("Leak más grande:"), 1,0);   kpi.addWidget(self.lbl_biggest, 1,1)
        kpi.addWidget(QLabel("Archivo con más leaks:"), 2,0); kpi.addWidget(self.lbl_file_most, 2,1)
        kpi.addWidget(QLabel("Tasa de leaks (%):"), 3,0); kpi.addWidget(self.lbl_rate, 3,1)
        box = QGroupBox("Resumen de leaks"); box.setLayout(kpi)

        # Gráficas
        self.bars = MplFigure("Leaks por archivo (MB)")
        self.pie = MplFigure("Distribución de leaks")

        lay = QVBoxLayout()
        lay.addWidget(box)
        lay.addWidget(self.bars)
        lay.addWidget(self.pie)
        self.setLayout(lay)

    @staticmethod
    def _mb(bytes_): return round(bytes_ / (1024*1024), 3)

    def refresh(self):
        # Distribución de leaks actual = memoria viva por archivo
        by_file_live = defaultdict(int)
        biggest = 0
        for a in self.model.live.values():
            by_file_live[a.file] += a.size
            if a.size > biggest: biggest = a.size

        total_leak = sum(by_file_live.values())
        self.lbl_total.setText(str(self._mb(total_leak)))
        self.lbl_biggest.setText(str(self._mb(biggest)))
        if by_file_live:
            file_most = max(by_file_live.items(), key=lambda kv: kv[1])[0]
            self.lbl_file_most.setText(file_most)
        else:
            self.lbl_file_most.setText("-")

        # tasa leaks = liveAllocs / totalAllocs * 100
        rate = 0.0
        if self.model.total_allocs > 0:
            rate = (len(self.model.live) / self.model.total_allocs) * 100.0
        self.lbl_rate.setText(f"{rate:.2f}")

        items = sorted(by_file_live.items(), key=lambda kv: kv[1], reverse=True)
        labels = [k if len(k)<30 else "..." + k[-27:] for k,_ in items[:10]]
        values = [self._mb(v) for _,v in items[:10]]
        self.bars.bar(labels, values, "archivo", "MB")
        self.pie.pie(labels, values)

# ----------- Ventana principal -----------
class MainWindow(QtWidgets.QMainWindow):
    def __init__(self, q):
        super().__init__()
        self.setWindowTitle("MemProf GUI")

        self.model = ProfilerModel()
        self.q = q

        self.tabs = QTabWidget()
        self.tab_overview = TabOverview(self.model)
        self.tab_map      = TabMemMap(self.model)
        self.tab_file     = TabByFile(self.model)
        self.tab_leaks    = TabLeaks(self.model)

        self.tabs.addTab(self.tab_overview, "Vista general")
        self.tabs.addTab(self.tab_map, "Mapa de memoria")
        self.tabs.addTab(self.tab_file, "Por archivo")
        self.tabs.addTab(self.tab_leaks, "Leaks")

        status = QWidget()
        s_lay = QHBoxLayout()
        self.lbl_conn = QLabel("Estado: esperando conexión...")
        s_lay.addWidget(self.lbl_conn); s_lay.addStretch(1)
        status.setLayout(s_lay)

        central = QWidget()
        v = QVBoxLayout()
        v.addWidget(self.tabs)
        v.addWidget(status)
        central.setLayout(v)
        self.setCentralWidget(central)

        # Timer para consumir eventos y refrescar GUI
        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.tick)
        self.timer.start(120)

    def tick(self):
        has_event = False
        while not self.q.empty():
            j = self.q.get()
            t = j.get("type", "")
            if t == "ALLOC":
                self.model.on_alloc(j); has_event = True
            elif t == "FREE":
                self.model.on_free(j); has_event = True
            elif t == "SNAPSHOT":
                self.model.on_snapshot(j); has_event = True
            elif t == "LEAK_SUMMARY":
                self.model.on_leak_summary(j); has_event = True

        if has_event:
            self.lbl_conn.setText(f"Eventos recibidos. Live={len(self.model.live)}  TotalAllocs={self.model.total_allocs}")
            self.tab_overview.refresh()
            self.tab_map.refresh()
            self.tab_file.refresh()
            self.tab_leaks.refresh()

# ----------- main() -----------
def main():
    app = QtWidgets.QApplication(sys.argv)
    q = queue.Queue()
    ReceiverThread(q).start()
    w = MainWindow(q)
    w.resize(1000, 720)
    w.show()
    sys.exit(app.exec())

if __name__ == "__main__":
    main()
