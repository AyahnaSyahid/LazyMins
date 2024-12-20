#ifndef ComunicatorH
#define ComunicatorH

#include <QObject>

class QTcpSocket;
class Comunicator : public QObject {
    Q_OBJECT
public:
    Comunicator(qintptr, QObject* =nullptr);
    bool begin();

public slots:
    void readData();

signals:
    void dataReceived(const QByteArray&);
    void requestDelete(const QByteArray&);
    void writeFinish(qint64);

private slots:
    void onWriteFinish(qint64);
    void onDisconnected();

private:
    QTcpSocket* socket;
};

#endif // ComunicatorH