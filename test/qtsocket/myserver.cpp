#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QByteArray>
#include <QDebug>
#include <QObject>

class MyServer : public QTcpServer {
    Q_OBJECT

public:
    MyServer(QObject *parent = nullptr) : QTcpServer(parent) {
        qDebug() << "MyServer - initialized";
    }

protected:
    void incomingConnection(qintptr socketDescriptor) override {
        qDebug() << "New connection received with descriptor" << socketDescriptor;
        emit clientConnection(socketDescriptor);
    }

signals:
    void clientConnection(qintptr);
};
