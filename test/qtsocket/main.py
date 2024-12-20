# from PyQt5.QtWidgets import 
from PyQt5.QtCore import QTimer, QCoreApplication

from myserver import MyServer
from clienthandler import ConnectionHandler, QHostAddress
from runnable import Runnable
from comunicator import Comunicator

BLAST = 0
def counter():
    global BLAST
    BLAST += 1
    return BLAST

if __name__ == "__main__":
    app = QCoreApplication([])
    server = MyServer()
    cl = ConnectionHandler()
    server.clientConnection['qintptr'].connect(cl.handleClient)
    server.listen(QHostAddress.AnyIPv4, 3400)
    fc = QTimer()
    fc.setInterval(20000)
    fc.setSingleShot(True)
    fc.timeout.connect(app.quit)
    
    fr = QTimer()
    fr.setInterval(1000)
    fr.timeout.connect(lambda : print(counter(), len(cl.comunicators)))
    fr.start()
    fc.start()
    app.exec()
    