import sqlite3
import sys
import os
from PyQt5.QtWidgets import QApplication, QFileDialog, QDialog, QVBoxLayout, QHBoxLayout, QFormLayout, QPushButton, QLineEdit, QLabel, QTextEdit
from PyQt5.QtCore import Qt, pyqtSlot

def copy_full_schema(source_db, target_db, log_callback):
    """
    Copy all schema objects (tables, views, indexes, triggers) from source SQLite database to target SQLite database.
    
    Args:
        source_db (str): Path to source database file
        target_db (str): Path to target database file
        log_callback (callable): Function to output progress messages
    """
    try:
        # Connect to source database
        log_callback("Connecting to source database...")
        source_conn = sqlite3.connect(source_db)
        source_cursor = source_conn.cursor()
        
        # Get all schema objects (tables, views, indexes, triggers) excluding system objects
        log_callback("Retrieving schema objects from source database...")
        source_cursor.execute("SELECT sql, name, type FROM sqlite_master WHERE sql IS NOT NULL AND name NOT LIKE 'sqlite_%';")
        schema_objects = source_cursor.fetchall()
        
        # Connect to target database
        log_callback("Connecting to target database...")
        target_conn = sqlite3.connect(target_db)
        target_cursor = target_conn.cursor()
        
        # Copy each schema object
        log_callback(f"Found {len(schema_objects)} schema objects to copy.")
        for schema_sql, name, obj_type in schema_objects:
            log_callback(f"Copying {obj_type} '{name}'...")
            try:
                target_cursor.execute(schema_sql)
                log_callback(f"Successfully copied {obj_type} '{name}'.")
            except sqlite3.Error as e:
                log_callback(f"Error copying {obj_type} '{name}': {e}")
                continue
        
        # Commit changes and close connections
        log_callback("Committing changes to target database...")
        target_conn.commit()
        log_callback("Closing database connections...")
        source_conn.close()
        target_conn.close()
        
        log_callback(f"All schema objects successfully copied from {source_db} to {target_db}")
        
    except sqlite3.Error as e:
        log_callback(f"SQLite error occurred: {e}")
    except Exception as e:
        log_callback(f"Unexpected error: {e}")

class GuiDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("Copy SQLite Schema")
        
        # Main layout
        main_layout = QVBoxLayout()
        
        # Form layout for input fields
        input_layout = QFormLayout()
        
        # Source database input
        self.lSource = QLabel("Source", self)
        self.iSource = QLineEdit(self)
        self.bSource = QPushButton("Pilih", self)
        ls = QHBoxLayout()
        ls.addWidget(self.iSource)
        ls.addWidget(self.bSource)
        input_layout.addRow(self.lSource, ls)
        
        # Destination database input
        self.lDest = QLabel("Dest", self)
        self.iDest = QLineEdit(self)
        self.bDest = QPushButton("Pilih", self)
        ld = QHBoxLayout()
        ld.addWidget(self.iDest)
        ld.addWidget(self.bDest)
        input_layout.addRow(self.lDest, ld)
        
        # Progress output
        self.lProgress = QLabel("Progress", self)
        self.tProgress = QTextEdit(self)
        self.tProgress.setReadOnly(True)
        input_layout.addRow(self.lProgress, self.tProgress)
        
        # Start button
        self.btn = QPushButton("Mulai", self)
        lb = QHBoxLayout()
        lb.addStretch()
        lb.addWidget(self.btn)
        
        # Add layouts to main layout
        main_layout.addLayout(input_layout)
        main_layout.addLayout(lb)
        self.setLayout(main_layout)
        
        # Connect signals
        self.bSource.clicked.connect(self.on_bSource_clicked)
        self.bDest.clicked.connect(self.on_bDest_clicked)
        self.btn.clicked.connect(self.on_mBtn_clicked)
        
    def log_progress(self, message):
        """Append a progress message to the QTextEdit widget."""
        self.tProgress.append(f"{message}")
        QApplication.processEvents()  # Ensure GUI updates immediately
        
    @pyqtSlot()
    def on_bSource_clicked(self):
        srcfile = QFileDialog.getOpenFileName(self, "Pilih database referensi", "", "SQLite Database (*.db *.sqlite *.sqlite3)")
        if srcfile[0]:
            self.iSource.setText(srcfile[0])
            self.log_progress(f"Selected source database: {srcfile[0]}")
        
    @pyqtSlot()
    def on_bDest_clicked(self):
        saveFile = QFileDialog.getSaveFileName(self, "Tentukan tempat penyimpanan", "", "SQLite Database (*.db *.sqlite *.sqlite3)")
        if saveFile[0]:
            self.iDest.setText(saveFile[0])
            self.log_progress(f"Selected destination database: {saveFile[0]}")
            
    @pyqtSlot()
    def on_mBtn_clicked(self):
        source = self.iSource.text()
        dest = self.iDest.text()
        if not source or not dest:
            self.log_progress("Error: Please select both source and destination databases.")
            return
        self.log_progress("Starting schema copy process...")
        copy_full_schema(source, dest, self.log_progress)
        self.log_progress("Schema copy process completed.")

if __name__ == "__main__":
    app = QApplication(sys.argv)
    win = GuiDialog()
    win.show()
    sys.exit(app.exec())