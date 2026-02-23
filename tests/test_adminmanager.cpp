#include "test_adminmanager.h"

#include <QSqlDatabase>

#include "src/database/databasemanager.h"
#include "src/managers/adminmanager.h"
#include "src/utils/authmanager.h"
 
void test_AdminManager::initTestCase() {
  auto db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName("D:/Project/LazyMins/a.db");
  if (!db.open()) {
        QFAIL("Gagal buka in-memory DB: ");
    }
  QSqlQuery q(db);
  q.exec("PRAGMA foreign_keys = ON");
  q.exec("DELETE FROM admins");
  DatabaseManager::instance().setDatabase(db);
  BaseManager::connection = db;
  
  QVariantMap noer {
        {"username",        "nurholis"},
        {"literal_password","nahalakamarada"},
        {"nama_lengkap",    "Nur Holis Komarudin"},
        {"role_id",         1}
  };

  QVariantMap maman {
      {"username",        "maman"},
      {"literal_password","123-apxy"},
      {"nama_lengkap",    "Maman Nurzaman"},
      {"role_id",         2}
  };

  QVariantMap syahid {
      {"username",        "syahid"},
      {"literal_password","budak-bapa"},
      {"nama_lengkap",    "Syahid Yusuf Nurdiansyah"},
      {"role_id",         3}
  };

  QVariantMap madun { // duplikat username
      {"username",        "maman"},
      {"literal_password","123-apxy"},
      {"nama_lengkap",    "Maman Nurzaman"},
      {"role_id",         2}
  };
  
  users << noer << maman << syahid << madun;
}

void test_AdminManager::testCreate_data()
{
    QTest::addColumn<QVariantMap>("userData");
    QTest::addColumn<bool>("expectedSuccess");
    
    auto addRow = [](const char* row, const QVariantMap &var, bool exp) {
        QTest::newRow(row) << var << exp;
    };

    addRow("noer_valid",      users[0],    true);
    addRow("maman_valid",     users[1],    true);
    addRow("syahid_valid",    users[2],    true);
    addRow("madun_duplicate", users[3],   false);
}

void test_AdminManager::testCreate()
{
    QFETCH(QVariantMap, userData);
    QFETCH(bool, expectedSuccess);

    AdminManager am;
    auto result = am.create(userData);

    QCOMPARE(result.has_value(), expectedSuccess);

    if (expectedSuccess) {
        QVERIFY(result.has_value());
        const QSqlRecord &rec = *result;
        QCOMPARE(rec.value("username").toString(),     userData["username"].toString());
        QCOMPARE(rec.value("nama_lengkap").toString(), userData["nama_lengkap"].toString());
        QCOMPARE(rec.value("role_id").toInt(),         userData["role_id"].toInt());
        qDebug().noquote() << QString("Record ID : %1, berhasil dibuat").arg(rec.value("id").toString());
    }
    // else → bisa ditambahkan QCOMPARE(!result.has_value(), true); sudah tercover oleh QCOMPARE pertama
}

void test_AdminManager::testLogin_data()
{
    QTest::addColumn<QStringList>("loginDetail");   // {username, password}
    QTest::addColumn<bool>("shouldPass");           // ekspektasi: true = login sukses

    // Asumsi: users adalah QVector<QVariantMap> atau QList<QVariantMap>
    // yang sudah diinisialisasi (lihat contoh di bawah jika belum ada)

    QTest::newRow("login_noer_valid")
        << QStringList{users[0]["username"].toString(), users[0]["literal_password"].toString()}
        << true;

    QTest::newRow("login_maman_valid")
        << QStringList{users[1]["username"].toString(), users[1]["literal_password"].toString()}
        << true;

    QTest::newRow("login_noer_wrong_password")
        << QStringList{users[0]["username"].toString(), QString("Gagal")}
        << false;

    QTest::newRow("login_maman_wrong_password")
        << QStringList{users[1]["username"].toString(), QString("FAIL")}
        << false;

    QTest::newRow("login_syahid_valid")
        << QStringList{users[2]["username"].toString(), users[2]["literal_password"].toString()}
        << true;

    QTest::newRow("login_syahid_wrong_password")
        << QStringList{users[2]["username"].toString(), QString("FAIL")}
        << false;

    // Opsional: tambahkan kasus edge case
    QTest::newRow("login_username_empty")
        << QStringList{"", "somepass"} << false;

    QTest::newRow("login_password_empty")
        << QStringList{"validuser", ""} << false;
}

void test_AdminManager::testLogin()
{
    QFETCH(QStringList, loginDetail);
    QFETCH(bool, shouldPass);

    // Pastikan data input valid (minimal 2 elemen)
    if (loginDetail.size() != 2) {
        QFAIL("Data login tidak lengkap: username dan password diperlukan");
    }

    const QString username = loginDetail.at(0);
    const QString password = loginDetail.at(1);

    auto &aum = AuthManager::instance();

    // Panggil fungsi login (sesuaikan dengan signature sebenarnya)
    bool success = aum.passwordMatch(username, password);
    aum.setCurrentAdmin(username);
    // Alternatif jika login menerima QStringList:
    // bool success = aum.login(loginDetail);

    QCOMPARE(success, shouldPass);

    // Opsional: verifikasi tambahan jika login sukses
    if (shouldPass) {
        // Misalnya: cek apakah current user sesuai
        QCOMPARE(aum.currentAdmin().value("username").toString(), username);
        // atau QVERIFY(aum.isAuthenticated());
    }
}
QTEST_MAIN(test_AdminManager);
