from PyQt5.QtCore import QThreadPool, QObject, pyqtSlot, QByteArray
from PyQt5.QtNetwork import QHostAddress

from runnable import Runnable

class ConnectionHandler(QObject):
    
    def __init__(self, parent=None):
        super().__init__(parent)
        self.comunicators = []
        print("ConnectionHandler - initialized...")
    
    @pyqtSlot('qintptr')
    def handleClient(self, qintptr):
        print("Creating Runnable")
        runnable = Runnable(qintptr, self)
        QThreadPool.globalInstance().start(runnable)

    @pyqtSlot(QByteArray)
    def onDataReceived(self, ba):
        print("Succeed received data")
        print(ba)
    
    @pyqtSlot()
    def removeMe(self):
        print("RequestDelete Received")
        sender = self.sender()
        ix = self.comunicators.index(sender)
        if ix and ix > -1:
            pop = self.comunicators.pop(ix)
        sender.deleteLater()