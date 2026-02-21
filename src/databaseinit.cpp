#include <QFile>
#include <QTextStream>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSettings>
#include <QDebug>

bool initializeDatabase()
{
    QSettings s;

    // Ambil path database dari pengaturan
    QString dbPath = s.value("Database/Path").toString();
    if (dbPath.isEmpty()) {
        qCritical() << "Database path is not set in QSettings (key: Database/Path)";
        return false;
    }

    // Pastikan koneksi SQLite
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "InitConnection");
    db.setDatabaseName(dbPath);

    if (!db.open()) {
        qCritical() << "Failed to open/create database file:" << dbPath;
        qCritical() << "Error:" << db.lastError().text();
        return false;
    }

    // Baca file skema dari resource
    QFile schemaFile(":/def/jinv.schema");
    if (!schemaFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Cannot open schema resource file: :/def/jinv.schema";
        db.close();
        return false;
    }

    QTextStream in(&schemaFile);
    QString content = in.readAll();
    schemaFile.close();

    // Pisah berdasarkan delimiter "XXXXXX" (harus satu baris penuh)
    QStringList statements = content.split("XXXXXX", Qt::SkipEmptyParts);

    QSqlQuery query(db);
    int executedCount = 0;
    int failedCount   = 0;

    // Nonaktifkan autocommit → gunakan transaksi besar
    db.transaction();

    for (QString statement : statements) {
        statement = statement.trimmed();
        if (statement.isEmpty()) {
            continue;
        }

        // Hapus komentar satu baris jika ada (opsional, tergantung format Anda)
        // statement = statement.remove(QRegularExpression("--.*"));

        if (!query.exec(statement)) {
            qWarning() << "Failed to execute statement:";
            qWarning() << statement.left(120) << (statement.length() > 120 ? "..." : "");
            qWarning() << "Error:" << query.lastError().text();
            failedCount++;
        } else {
            executedCount++;
        }
    }

    if (failedCount > 0) {
        db.rollback();
        qCritical() << "Initialization rolled back due to" << failedCount << "error(s)";
        qCritical() << executedCount << "statements executed successfully before failure";
        db.close();
        return false;
    }

    if (!db.commit()) {
        qCritical() << "Failed to commit transaction:" << db.lastError().text();
        db.close();
        return false;
    }

    db.close();

    qInfo() << "Database initialized successfully";
    qInfo() << executedCount << "statements executed";
    qInfo() << "Database file:" << dbPath;

    return true;
}