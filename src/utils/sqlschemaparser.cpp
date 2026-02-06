#include "sqlschemaparser.h"

SqlSchemaParser::SqlSchemaParser(QObject *parent)
    : QObject(parent)
{
    resetState();
}

bool SqlSchemaParser::parseFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        lastError = QString("Cannot open file: %1").arg(filePath);
        return false;
    }

    QTextStream stream(&file);
    return parseStream(stream);
}

bool SqlSchemaParser::parseResource(const QString &resourcePath)
{
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        lastError = QString("Cannot open resource: %1").arg(resourcePath);
        return false;
    }

    QTextStream stream(&file);
    return parseStream(stream);
}

bool SqlSchemaParser::parseStream(QTextStream &stream)
{
    resetState();

    QString line;

    while (!stream.atEnd()) {
        line = stream.readLine().trimmed();
        if (line.startsWith("--")) {
          
        }
    }
}

QStringList SqlSchemaParser::getStatements() const
{
    return statements;
}

QString SqlSchemaParser::getLastError() const
{
    return lastError;
}

bool SqlSchemaParser::hasError() const
{
    return !lastError.isEmpty();
}

void SqlSchemaParser::resetState()
{
    statements.clear();
    currentStatement.clear();
    currentContext.clear();
    insideMultiLine = false;
    inMultiLineComment = false;
    lastError.clear();
}

bool SqlSchemaParser::isCommentLine(const QString &line) const
{
    return reSingleLineComment.match(line).hasMatch();
}

QString SqlSchemaParser::cleanStatement(const QString &stmt) const
{
    QString s = stmt.trimmed();

    // Remove trailing semicolon
    if (s.endsWith(';')) {
        s.chop(1);
        s = s.trimmed();
    }

    // Very simple trailing -- comment removal (after last non-string ;)
    qsizetype pos = s.lastIndexOf("--");
    if (pos > 0) {
        // Very naive — improve later if needed
        // For production you should scan properly skipping strings
        s = s.left(pos).trimmed();
    }

    return s;
}

bool SqlSchemaParser::containsUnescapedSemicolon(const QString &text) const
{
    bool inSingle = false;
    bool inDouble = false;

    for (QChar c : text) {
        if (c == '\'' && !inDouble) {
            inSingle = !inSingle;
            continue;
        }
        if (c == '"' && !inSingle) {
            inDouble = !inDouble;
            continue;
        }

        if (!inSingle && !inDouble && c == ';') {
            return true;
        }
    }

    return false;
}