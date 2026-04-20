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
  int caseDepth = 0;
  QString statement, line;

  while (!ts.atEnd()) {
    line = ts.readLine();

    if (line.trimmed().isEmpty()) continue;
    if (line.trimmed().startsWith("--")) continue;

    if (line.toLower().contains("create trigger")) {
      insideCreateTrigger = true;
      caseDepth = 0;
      statement += "\n" + line;
      continue;
    }

    if (insideCreateTrigger) {
      QString stripped = line.trimmed().toLower();

      if (stripped.startsWith("case")) {
        caseDepth++;
      }

      statement += "\n" + line;

      if (stripped.contains("end;")) {
        if (caseDepth > 0) {
          caseDepth--;  // END; milik CASE
        } else {
          insideCreateTrigger = false;  // END; milik trigger
          stl << statement.trimmed();
          statement.clear();
        }
      }
      continue;
    }

    if (line.contains(";")) {
      statement += "\n" + line;
      stl << statement.trimmed();
      statement.clear();
      continue;
    }

    statement += "\n" + line;
  }

  sf.close();

  // Flush statement terakhir jika ada
  if (!statement.trimmed().isEmpty()) {
    stl << statement.trimmed();
  }

  qDebug() << "Using :" << db.databaseName();
  qDebug() << "Total statements:" << stl.size();

  if (!db.transaction()) {
    qDebug() << "Failed to start transaction:" << db.lastError().text();
    return false;
  }

  QSqlQuery q(db);
  for (const auto& st : stl) {
    if (!q.exec(st)) {
      qDebug() << "Query error:" << q.lastError().text();
      qDebug() << st;
      db.rollback();
      return false;
    }
  }

  if (!db.commit()) {
    qDebug() << "Commit failed:" << db.lastError().text();
    db.rollback();
    return false;
  }

  return true;
}