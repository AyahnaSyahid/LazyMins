#include "src/database/initializeschema.h"
#include <QCoreApplication>
#include <QString>
#include <QSqlDatabase>

int main(int argc, char **args) {
  
  QCoreApplication app(argc, args);
  for(int a=0; a<argc; ++a) {
    qDebug().noquote() << QString("%1. %2").arg(QString::number(a + 1), QString(args[a]));
  }
  
  QString sql = args[1];
  QString db_file = args[2];
  
  auto base = QSqlDatabase::addDatabase("QSQLITE");
  base.setDatabaseName(db_file);
  base.open();
  
  if (initializeSchemaFile(sql, base))
    qDebug().noquote() << "Berhasil";
  else
    qDebug().noquote() << "Gagal";
  
  return 0;
}