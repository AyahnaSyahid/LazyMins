#pragma once

#include <QSqlDatabase>

bool initializeSchemaFile(const QString& schema, QSqlDatabase db = QSqlDatabase::database());