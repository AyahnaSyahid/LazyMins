#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QtDebug>

int execOrExit(const char* que, QSqlQuery* q) {
    if(!q->exec(que)) {
        qDebug() << q->lastError() ;
        return 0;
    }
    return 1;
}

int main(int argc, char** argv) {
    QFile dbr(":/db/base.db3");
    for(int i=0; i<argc; ++i) {
        qDebug() << QString("Argument [%1] = '%2'").arg(i).arg(QString(argv[i]));
    }
    dbr.copy("source_base");
    QSqlDatabase target = QSqlDatabase::addDatabase("QSQLITE", "target");
    target.setDatabaseName(argv[1]);
    
    QSqlDatabase base = QSqlDatabase::addDatabase("QSQLITE", "source");
    base.setDatabaseName("source_base");
    
    target.open();
    base.open();
    
    QSqlQuery qs(base);
    QSqlQuery qt(target);
    
    qt.exec("PRAGMA foreign_keys = OFF;");
    qt.exec("BEGIN");
    QStringList typeSeq {"table", "view", "trigger"};
    int ok = 0;
    for(auto ttype = typeSeq.cbegin(); ttype != typeSeq.cend(); ++ttype) {
        qs.exec(QString("SELECT tbl_name, sql FROM sqlite_master WHERE [type] = '%1'").arg(*ttype));
        while(qs.next()) {
            qDebug() << QString("Creating %1 : %2").arg(*ttype, qs.value("tbl_name").toString());
            if(qt.exec(qs.value("sql").toString())) {
                qDebug() << "Created" << qs.value("tbl_name").toString();
            } else {
                qDebug() << qt.lastError();
            }
        }
    }
    if(qt.exec("COMMIT;")) {
        qDebug() << "Copy Structure Done!"; 
    } else {
        qDebug() << qt.lastError(); 
    }
    QFile::remove("source_base");
}
