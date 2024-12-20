#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThreadPool>
#include "MyServer.cpp"
#include "ConnectionHandler.cpp"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    MyServer server;
    ConnectionHandler handler;

    QObject::connect(&server, &MyServer::clientConnection, &handler, &ConnectionHandler::handleClient);
    
    if (server.listen(QHostAddress::Any, 3400)) {
        qDebug() << "Server started on port 3400";
    } else {
        qDebug() << "Server failed to start";
        return -1;
    }

    return a.exec();
}
