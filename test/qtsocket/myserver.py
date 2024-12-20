from PyQt5.QtNetwork import QTcpServer, QHostAddress
from PyQt5.QtCore import pyqtSignal

class MyServer(QTcpServer):
    clientConnection = pyqtSignal('qintptr')
    
    def __init__(self, parent=None):
        super().__init__(parent)
        print("MyServer - initialized")
    
    def incomingConnection(self, intptr):
        self.clientConnection.emit(intptr)