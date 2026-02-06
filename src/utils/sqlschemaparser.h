#ifndef SQLSCHEMAPARSER_H
#define SQLSCHEMAPARSER_H

#include <QObject>
#include <QStringList>
#include <QRegularExpression>
#include <QFile>
#include <QTextStream>

class SqlSchemaParser : public QObject
{
    Q_OBJECT

public:
    explicit SqlSchemaParser(QObject *parent = nullptr);
    ~SqlSchemaParser() = default;

    bool parseFile(const QString &filePath);
    bool parseResource(const QString &resourcePath);
    bool parseStream(QTextStream& ts);

    QStringList getStatements() const;
    QString getLastError() const;
    bool hasError() const;

private:
    void resetState();
    QString cleanStatement(const QString &stmt) const;
    bool isCommentLine(const QString &line) const;
    bool containsUnescapedSemicolon(const QString &text) const;

    // State
    QStringList statements;
    QString currentStatement;
    QString currentContext;           // "trigger", "view", "insert", ""
    bool insideMultiLine = false;
    bool inMultiLineComment = false;

    // Regular expressions (compiled once)
    const QRegularExpression reTriggerStart{
        R"(^\s*CREATE\s+TRIGGER\s+)", QRegularExpression::CaseInsensitiveOption
    };
    const QRegularExpression reViewStart{
        R"(^\s*CREATE\s+(?:TEMP\s+|TEMPORARY\s+)?VIEW\s+)", QRegularExpression::CaseInsensitiveOption
    };
    const QRegularExpression reInsertStart{
        R"(^\s*INSERT\s+(?:OR\s+[A-Z]+\s+)?INTO\s+)", QRegularExpression::CaseInsensitiveOption
    };
    const QRegularExpression reSingleLineComment{ R"(^\s*--)" };
    const QRegularExpression reMultiCommentStart{ R"(/\*)" };
    const QRegularExpression reMultiCommentEnd{ R"(\*/)" };

    QString lastError;
};

#endif // SQLSCHEMAPARSER_H