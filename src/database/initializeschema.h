#pragma once

#include <QSqlDatabase>

bool initializeSchemaFile(const QString& schema, const QSqlDatabase &db = QSqlDatabase::database());