import sys
from PyQt6.QtWidgets import QApplication, QWidget, QVBoxLayout, QTextEdit
from PyQt6.QtGui import QFont

class EscPosMinimalRenderer(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Minimal ESC/POS Renderer")
        self.resize(350, 500)
        
        # Setup Layout dan Widget Teks
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        
        self.text_display = QTextEdit(self)
        self.text_display.setReadOnly(True)
        # Gunakan font Monospace agar menyerupai hasil cetakan printer struk (thermal)
        self.text_display.setFont(QFont("Courier New", 10))
        # Warna background seperti kertas struk
        self.text_display.setStyleSheet("background-color: #FDFDFD; color: black;") 
        
        layout.addWidget(self.text_display)

    def render_escpos(self, byte_data: bytes):
        """
        Parser ESC/POS super minimal.
        Hanya mendukung:
        - Teks biasa
        - LF (0x0A) -> Baris baru
        - ESC E (0x1B 0x45) -> Bold on/off
        - ESC a (0x1B 0x61) -> Alignment (Kiri, Tengah, Kanan)
        """
        html_output = ""
        i = 0
        
        is_bold = False
        align = "left"
        
        # Mulai blok HTML pertama
        html_output += f"<div style='text-align: {align};'>"

        while i < len(byte_data):
            byte = byte_data[i]
            
            # [1] Line Feed (Baris Baru)
            if byte == 0x0A: 
                html_output += "<br>"
                i += 1
                
            # [2] Karakter Escape (Perintah)
            elif byte == 0x1B: 
                if i + 1 < len(byte_data):
                    next_byte = byte_data[i+1]
                    
                    # ESC @ (Initialize Printer)
                    if next_byte == 0x40: 
                        is_bold = False
                        align = "left"
                        i += 2
                        continue
                        
                    # ESC E n (Turn emphasized mode on/off)
                    elif next_byte == 0x45 and i + 2 < len(byte_data):
                        is_bold = bool(byte_data[i+2])
                        i += 3
                        continue
                        
                    # ESC a n (Select justification)
                    elif next_byte == 0x61 and i + 2 < len(byte_data):
                        val = byte_data[i+2]
                        # Tutup div sebelumnya, buka yang baru dengan alignment baru
                        html_output += "</div>"
                        if val == 0 or val == 48: align = "left"
                        elif val == 1 or val == 49: align = "center"
                        elif val == 2 or val == 50: align = "right"
                        html_output += f"<div style='text-align: {align};'>"
                        i += 3
                        continue
                        
                # Jika perintah ESC tidak dikenali, lewati saja karakter ESC-nya
                i += 1
                
            # [3] Karakter Teks Biasa (ASCII)
            else:
                try:
                    # Render karakter sebagai string
                    char = bytes([byte]).decode('cp437') # Printer thermal biasanya pakai code page 437
                    
                    # Escape karakter khusus HTML
                    if char == '<': char = "&lt;"
                    elif char == '>': char = "&gt;"
                    elif char == '&': char = "&amp;"
                    elif char == ' ': char = "&nbsp;" # Jaga spasi monospace
                    
                    if is_bold:
                        html_output += f"<b>{char}</b>"
                    else:
                        html_output += char
                except UnicodeDecodeError:
                    pass # Abaikan karakter biner yang tidak bisa di-decode
                i += 1
                
        html_output += "</div>"
        
        # Set hasil HTML ke dalam QTextEdit
        self.text_display.setHtml(html_output)


# --- CONTOH PENGGUNAAN ---
if __name__ == "__main__":
    app = QApplication(sys.argv)
    
    # Simulasi data bytes dari perintah ESC/POS
    # Format: [Init] [Center] [Bold On] TOKO KITA [LF] [Bold Off] [Left] Teks biasa [LF]
    sample_escpos_data = b"".join([
        b"\x1B\x40",             # ESC @ (Init)
        b"\x1B\x61\x01",         # ESC a 1 (Center Align)
        b"\x1B\x45\x01",         # ESC E 1 (Bold On)
        b"TOKO BINTANG JAYA\n",  # Teks + LF
        b"\x1B\x45\x00",         # ESC E 0 (Bold Off)
        b"Jalan Sudirman No. 123\n",
        b"\n",
        b"\x1B\x61\x00",         # ESC a 0 (Left Align)
        b"Item 1             Rp 10.000\n",
        b"Item 2             Rp 20.000\n",
        b"------------------------------\n",
        b"\x1B\x61\x02",         # ESC a 2 (Right Align)
        b"Total: Rp 30.000\n",
        b"\n\n\n"
    ])

    widget = EscPosMinimalRenderer()
    widget.render_escpos(sample_escpos_data)
    widget.show()
    
    sys.exit(app.exec())