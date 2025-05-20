import sqlite3
import sys
import os
import sqlite3
import os

def copy_full_schema(source_db, target_db):
    """
    Copy all schema objects (tables, views, indexes, triggers) from source SQLite database to target SQLite database.
    
    Args:
        source_db (str): Path to source database file
        target_db (str): Path to target database file
    """
    try:
        # Connect to source database
        source_conn = sqlite3.connect(source_db)
        source_cursor = source_conn.cursor()
        
        # Get all schema objects (tables, views, indexes, triggers) excluding system objects
        source_cursor.execute("SELECT sql FROM sqlite_master WHERE sql IS NOT NULL AND name NOT LIKE 'sqlite_%';")
        schema_objects = source_cursor.fetchall()
        
        # Connect to target database
        target_conn = sqlite3.connect(target_db)
        target_cursor = target_conn.cursor()
        
        # Copy each schema object
        for schema_sql in schema_objects:
            sql_statement = schema_sql[0]
            try:
                target_cursor.execute(sql_statement)
            except sqlite3.Error as e:
                print(f"Error executing SQL: {sql_statement}\nError message: {e}")
                continue
        
        # Commit changes and close connections
        target_conn.commit()
        source_conn.close()
        target_conn.close()
        
        print(f"All schema objects successfully copied from {source_db} to {target_db}")
        
    except sqlite3.Error as e:
        print(f"An error occurred: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")


from PyQt5.QtWidgets import QApplication, QFileDialog, QDialog, QVBoxLayout, QHBoxLayout, QFormLayout, QPushButton, QLineEdit, QLabel
from PyQt5.QtCore import Qt, pyqtSlot, pyqtSignal, QMetaObject

class GuiDialog(QDialog):
    def __init__(self, parent=None):
        super().__init__(parent)
        inputLayout = QFormLayout()
        self.lSource = QLabel("Source", self)
        self.lDest = QLabel("Dest", self)
        self.iSource = QLineEdit(self)
        self.iDest = QLineEdit(self)
        self.bSource = QPushButton("Pilih", self)
        self.bDest = QPushButton("Pilih", self)
        self.btn = QPushButton("Mulai", self)
        ls = QHBoxLayout()
        ls.addWidget(self.iSource)
        ls.addWidget(self.bSource)
        ld = QHBoxLayout()
        ld.addWidget(self.iDest)
        ld.addWidget(self.bDest)
        inputLayout.addRow(self.lSource, ls)
        inputLayout.addRow(self.lDest, ld)
        self.setLayout(inputLayout)
        self.bSource.clicked.connect(self.on_bSource_clicked)
        self.bDest.clicked.connect(self.on_bDest_clicked)
        self.btn.clicked.connect(self.on_mBtn_clicked)
        lb = QHBoxLayout()
        lb.addStretch()
        lb.addWidget(self.btn)
        inputLayout.addRow(lb)
        
        
    @pyqtSlot()
    def on_bSource_clicked(self):
        srcfile = QFileDialog.getOpenFileName(self, "Pilih database referensi")
        if(len(srcfile[0])):
            self.iSource.setText(srcfile[0])
        
    @pyqtSlot()
    def on_bDest_clicked(self):
        saveFile = QFileDialog.getSaveFileName(self, "Tentukan tempat penyimpanan")
        if(len(saveFile[0])):
            self.iDest.setText(saveFile[0])
            
    @pyqtSlot()
    def on_mBtn_clicked(self):
        copy_full_schema(self.iSource.text(), self.iDest.text())
    
if __name__ == "__main__":
    app = QApplication([])
    win = GuiDialog()
    win.open()
    app.exec()