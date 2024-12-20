#include <QObject>
#include <QThreadPool>
#include <QByteArray>
#include <QTcpSocket>
#include <QList>
#include <QMutex>

#include "comunicator.h"

Comunicator::Comunicator(qintptr socketDescriptor, QObject *parent = nullptr) 
    : QObject(parent), socket(new QTcpSocket(this)) {
    if (!socket->setSocketDescriptor(socketDescriptor)) {
        qDebug() << "Failed to set socket descriptor.";
        return;
    }
    qDebug() << "Comunicator - initialized";
    
    connect(socket, &QTcpSocket::readyRead, this, &Comunicator::readData);
    connect(socket, &QTcpSocket::bytesWritten, this, &Comunicator::onWriteFinish);
    connect(socket, &QTcpSocket::disconnected, this, &Comunicator::onDisconnected);
}

bool Comunicator::begin() {
    socket->waitForReadyRead(3000);  // wait for data to be ready
    return socket->bytesAvailable() > 0;
}

void Comunicator::readData() {
    QByteArray data = socket->readAll();
    qDebug() << "Received data:" << data;
    emit dataReceived(data);
}

void Comunicator::sendData(const QByteArray &data) {
    socket->write(data);
}

void Comunicator::onWriteFinish(qint64 bytes) {
    qDebug() << "Data written:" << bytes;
}

void Comunicator::onDisconnected() {
    qDebug() << "Socket disconnected";
    emit requestDelete();
}