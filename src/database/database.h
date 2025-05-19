#ifndef Database_H
#define Database_H

#ifndef USER_ACCOUNT_DEFAULT_PASSWORD
#define USER_ACCOUNT_DEFAULT_PASSWORD "000000"
#endif

#include <QObject>
#include <QMap>

class QSqlTableModel;
class Database : public QObject
{
    Q_OBJECT

public:
    Database(QObject* = nullptr);
    ~Database();

    QSqlTableModel* getTableModel(const QString&);

signals:
    void paymentRequest(int inv);
    
private:
    QMap<QString, QSqlTableModel*> tModels;
};

#endif