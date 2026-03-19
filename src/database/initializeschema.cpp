#include "initializeschema.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>


bool initializeSchemaFile(const QString& fileName, QSqlDatabase db) {
  if (!db.isOpen()) return false;
  QStringList stl;
  QFile sf(fileName);
  if (!sf.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return false;
  }
  QTextStream ts(&sf);
  bool insideCreateTrigger = false;
  QString statement, line;
  while (!ts.atEnd()) {
    line = ts.readLine();
    if (line.isEmpty()) {
      continue;
    }
    if (line.toLower().contains("create trigger")) {
      insideCreateTrigger = true;
      statement += "\n" + line;
      continue;
    }
    if (line.contains(";")) {
      statement += "\n" + line;
      if (insideCreateTrigger) {
        if (line.toLower().contains("end;")) {
          insideCreateTrigger = false;
          stl << statement;
          statement.clear();
          continue;
        }
        continue;
      } else {
        stl << statement;
        statement.clear();
      }
      continue;
    }
    statement += "\n" + line;
  }
  qDebug() << "Using :" << db.databaseName();
  QSqlQuery q(db);
  for (auto st : stl) {
    q.exec(st);
  }
  return true;
};