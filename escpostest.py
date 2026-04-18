import sys
from PyQt6.QtWidgets import QApplication, QWidget, QVBoxLayout, QPushButton, QTextEdit
from PyQt6.QtGui import QPainter, QFont, QPen, QColor, QImage
from PyQt6.QtCore import Qt, QRect

class EscPosRenderer(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setMinimumSize(400, 600)
        self.lines = []          # List of rendered lines
        self.current_line = ""
        self.alignment = Qt.AlignmentFlag.AlignLeft
        self.font_size = 12      # Base font size
        self.bold = False
        self.underline = False
        self.double_width = False
        self.double_height = False
        self.reset_state()

    def reset_state(self):
        """ESC @ - Initialize printer"""
        self.current_line = ""
        self.alignment = Qt.AlignmentFlag.AlignLeft
        self.font_size = 12
        self.bold = False
        self.underline = False
        self.double_width = False
        self.double_height = False

    def parse_and_render(self, data: bytes):
        """Parse byte stream ESC/POS dan render ke layar (minimal)"""
        self.reset_state()
        self.lines.clear()
        i = 0
        while i < len(data):
            if data[i] == 0x1B:  # ESC
                if i + 1 < len(data):
                    cmd = data[i + 1]
                    if cmd == 0x40:  # ESC @ Initialize
                        self.reset_state()
                        i += 2
                        continue
                    elif cmd == 0x61:  # ESC a n - Alignment
                        if i + 2 < len(data):
                            n = data[i + 2]
                            if n == 0:
                                self.alignment = Qt.AlignmentFlag.AlignLeft
                            elif n == 1:
                                self.alignment = Qt.AlignmentFlag.AlignCenter
                            elif n == 2:
                                self.alignment = Qt.AlignmentFlag.AlignRight
                            i += 3
                            continue
                    elif cmd == 0x45:  # ESC E n - Emphasized (bold)
                        if i + 2 < len(data):
                            self.bold = bool(data[i + 2])
                            i += 3
                            continue
                    elif cmd == 0x47:  # ESC G n - Double strike (simulasi bold)
                        if i + 2 < len(data):
                            self.bold = bool(data[i + 2])
                            i += 3
                            continue
                    elif cmd == 0x2D:  # ESC - n - Underline
                        if i + 2 < len(data):
                            self.underline = bool(data[i + 2])
                            i += 3
                            continue
                    elif cmd == 0x21:  # ESC ! n - Print mode (simplified)
                        if i + 2 < len(data):
                            n = data[i + 2]
                            self.bold = bool(n & 0x08)
                            self.double_height = bool(n & 0x10)
                            self.double_width = bool(n & 0x20)
                            i += 3
                            continue
            elif data[i] == 0x0A:  # LF - Line Feed
                self._add_line()
                i += 1
                continue
            elif data[i] == 0x0D:  # CR
                i += 1
                continue
            elif data[i] == 0x1D:  # GS
                if i + 1 < len(data) and data[i + 1] == 0x21:  # GS ! n - Character size
                    if i + 2 < len(data):
                        n = data[i + 2]
                        self.double_width = bool(n & 0x10)
                        self.double_height = bool(n & 0x20)
                        i += 3
                        continue
            else:
                # Teks biasa (ASCII / Latin-1 untuk simplifikasi)
                self.current_line += chr(data[i])
            i += 1

        if self.current_line:
            self._add_line()

        self.update()  # Trigger repaint

    def _add_line(self):
        """Tambahkan baris saat ini ke daftar lines"""
        self.lines.append({
            'text': self.current_line,
            'alignment': self.alignment,
            'bold': self.bold,
            'underline': self.underline,
            'double_width': self.double_width,
            'double_height': self.double_height
        })
        self.current_line = ""

    def paintEvent(self, event):
        """Render ke widget menggunakan QPainter"""
        painter = QPainter(self)
        painter.fillRect(self.rect(), QColor(255, 255, 240))  # Warna kertas thermal

        y = 20
        line_height = 20
        width = self.width() - 40

        for line in self.lines:
            font = QFont("Courier New", self.font_size)
            if line['bold']:
                font.setBold(True)
            if line['double_height']:
                font.setPointSize(self.font_size * 2)
            painter.setFont(font)

            # Hitung lebar teks (simulasi double width)
            scale_x = 2 if line['double_width'] else 1
            text = line['text']
            if line['double_width']:
                text = ''.join(c + ' ' for c in text).strip()  # Simulasi sederhana

            # Alignment
            flags = line['alignment'] | Qt.TextFlag.TextDontClip
            rect = QRect(20, y, width, line_height * (2 if line['double_height'] else 1))

            painter.drawText(rect, flags, text)

            # Underline
            if line['underline']:
                pen = QPen(QColor(0, 0, 0), 1)
                painter.setPen(pen)
                underline_y = y + line_height * (2 if line['double_height'] else 1) - 2
                painter.drawLine(20, underline_y, 20 + painter.fontMetrics().horizontalAdvance(text) * scale_x, underline_y)

            y += line_height * (2 if line['double_height'] else 1) + 5

        # Border kertas
        painter.setPen(QPen(QColor(200, 200, 200), 2))
        painter.drawRect(10, 10, self.width() - 20, y + 20)


# ====================== Contoh Penggunaan ======================

class MainWindow(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Minimal ESC/POS Renderer")
        layout = QVBoxLayout(self)

        self.renderer = EscPosRenderer()
        layout.addWidget(self.renderer)

        btn = QPushButton("Render Contoh ESC/POS")
        btn.clicked.connect(self.load_example)
        layout.addWidget(btn)

        self.text_input = QTextEdit()
        self.text_input.setPlaceholderText("Masukkan byte ESC/POS dalam format hex (contoh: 1B 40 48 65 6C 6C 6F ...)")
        layout.addWidget(self.text_input)

    def load_example(self):
        # Contoh data ESC/POS sederhana (dalam bytes)
        example = (
            b'\x1B\x40'                     # ESC @ Initialize
            b'\x1B\x61\x01'                 # ESC a 1 -> Center
            b'\x1B\x21\x30'                 # ESC ! 0x30 -> Double width & height
            b'Halo, Ini Tes ESC/POS\r\n'
            b'\x1B\x61\x00'                 # ESC a 0 -> Left
            b'\x1B\x2D\x01'                 # ESC - 1 -> Underline on
            b'Teks dengan underline\r\n'
            b'\x1B\x2D\x00'                 # Underline off
            b'\x1B\x45\x01'                 # ESC E 1 -> Bold
            b'Teks Bold\r\n'
            b'\x1B\x45\x00'                 # Bold off
            b'\x0A'                         # LF
        )
        self.renderer.parse_and_render(example)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = MainWindow()
    window.resize(450, 700)
    window.show()
    sys.exit(app.exec())