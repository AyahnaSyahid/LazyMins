from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot, QByteArray
from PyQt5.QtNetwork import QTcpSocket

class Comunicator(QObject):
    dataReceived = pyqtSignal(QByteArray)
    writeFinish = pyqtSignal('qint64')
    requestDelete = pyqtSignal()

    def __init__(self, sd, parent=None):
        super().__init__(parent)
        self.sd = sd
        self.socket = QTcpSocket(self)
        self.socket.readyRead.connect(self.readData)
        self.socket.bytesWritten['qint64'].connect(self.writeFinish.emit)
        self.socket.disconnected.connect(self.requestDelete.emit)
        print("Comunicator - initialized")

    @pyqtSlot()
    def begin(self):
        if not self.socket.setSocketDescriptor(self.sd):
            return False
        print('Socket descriptor sets')
        self.socket.waitForReadyRead(3000)
        return True
    
    @pyqtSlot()
    def readData(self):
        print('read Data called')
        data = self.socket.readAll()
        print("data :", data)
        self.dataReceived.emit(QByteArray(data))
        # self.requestDelete.emit()
   
    @pyqtSlot(str,QByteArray)
    def sendData(self, ch, data):
        self.socket.write(data)