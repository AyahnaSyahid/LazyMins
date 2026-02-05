#include <QApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QtDebug>

int main(int argc, char **args)
{
  QApplication app(argc, args);
  
  // just for now, connect the to the database inside build/data directory
  auto db = QSqlDatabase::addDatabase("QSQLITE", "LMDatabase");
  db.setDatabaseName(QString("%1/data/lm.db3").arg(app.applicationDirPath()));
  if(!db.open()) {
    qDebug() << "Database Open Failed : " << db.databaseName();
    qDebug() << db.lastError().text();
    app.quit();
    return 0;
  }
  qDebug() << "Database :" << db.databaseName() << "Opened gracefully";
  app.quit();
  return 0;
};