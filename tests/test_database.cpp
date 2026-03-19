#include "src/database/initializeschema.h"
#include <QCoreApplication>
#include <QString>
#include <QSqlDatabase>

int main(int argc, char **args) {
  QCoreApplication app(argc, args);
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