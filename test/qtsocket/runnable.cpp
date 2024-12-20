#include <QRunnable>
#include <QByteArray>
#include <QTcpSocket>
#include <QThreadPool>

class Runnable : public QRunnable {
public:
    Runnable(qintptr socketDescriptor, ConnectionHandler *handler)
        : sockDesc(socketDescriptor), connectionHandler(handler) {}

    void run() override {
        qDebug() << "Runnable started";
        Comunicator *com = new Comunicator(sockDesc);
        if (!com->begin()) {
            qDebug() << "Couldn't begin Comunicator";
            return;
        }

        connect(com, &Comunicator::dataReceived, connectionHandler, &ConnectionHandler::onDataReceived);
        connect(com, &Comunicator::requestDelete, connectionHandler, &ConnectionHandler::removeMe);
    }

private:
    qintptr sockDesc;
    ConnectionHandler *connectionHandler;
};
