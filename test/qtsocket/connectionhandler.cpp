class ConnectionHandler : public QObject {
    Q_OBJECT

public:
    ConnectionHandler(QObject *parent = nullptr) : QObject(parent) {
        qDebug() << "ConnectionHandler - initialized";
    }

public slots:
    void handleClient(qintptr socketDescriptor) {
        qDebug() << "Creating Comunicator for socket:" << socketDescriptor;
        Comunicator *com = new Comunicator(socketDescriptor);
        if (!com->begin()) {
            qDebug() << "Couldn't begin Comunicator";
            return;
        }
        connect(com, &Comunicator::dataReceived, this, &ConnectionHandler::onDataReceived);
        connect(com, &Comunicator::requestDelete, this, &ConnectionHandler::removeMe);
        comunicators.append(com);
    }

    void onDataReceived(QByteArray data) {
        qDebug() << "Data received:" << data;
    }

    void removeMe() {
        Comunicator *sender = qobject_cast<Comunicator *>(sender());
        if (sender) {
            comunicators.removeAll(sender);
            sender->deleteLater();
        }
    }

private:
    QList<Comunicator *> comunicators;
};
