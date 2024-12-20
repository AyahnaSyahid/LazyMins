#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QVariant>
#include <QtDebug>

int main(int argc, char** argv) {
    auto base = QSqlDatabase::addDatabase("QSQLITE");
    base.setDatabaseName(":memory:");
    base.open();
    auto q = QSqlQuery("CREATE TABLE test_date ("
                         "local TIMESTAMP, utc TIMESTAMP, "
                         "internal_ts, internal_dt);");
    q.prepare("INSERT INTO test_date VALUES (?, ?, CURRENT_TIMESTAMP, datetime('now', 'localtime'));");
    q.addBindValue(QDateTime::currentDateTime());
    q.addBindValue(QDateTime::currentDateTimeUtc());
    q.exec();
    if(q.lastError().isValid()) {
        qDebug() << __FILE__ << __LINE__;
        qDebug() << q.lastError().text();
    }
    q.prepare("SELECT * FROM test_date WHERE ROWID = 1");
    q.exec();
    if(q.lastError().isValid()) {
        qDebug() << __FILE__ << __LINE__;
        qDebug() << q.lastError().text();
    }
    QVariantList val;
    if(q.next()) {
        for(int i=0; i<4; ++i) {
            qDebug() << "Column" << i;
            qDebug() << q.value(i);
            qDebug() << q.value(i).toDateTime();
            qDebug() << q.value(i).toDateTime().toUTC();
            val << q.value(i);
        }
    }
    qDebug() << QDateTime::currentDateTimeUtc();
}