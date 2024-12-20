from PyQt5.QtCore import QRunnable, QByteArray
from PyQt5.QtNetwork import QTcpSocket

from comunicator import Comunicator


class Runnable(QRunnable):
    
    canSafeDelete = False
    
    def __init__(self, sockd, cl):
        super().__init__()
        self.d = sockd
        self.cl = cl
        print("Runnable - initialized")
        
    def run(self):
        com = Comunicator(self.d)
        if not com.begin():
            print( "Coldnt Begin")
            self.canSafeDelete = True
            return
        com.dataReceived[QByteArray].connect(self.cl.onDataReceived)
        com.requestDelete.connect(self.cl.removeMe)
        self.cl.comunicators.append(com)