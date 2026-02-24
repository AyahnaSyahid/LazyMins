#include "test_adminmanager.h"

#include <QSqlDatabase>

#include "src/database/databasemanager.h"
#include "src/managers/adminmanager.h"
#include "src/utils/authmanager.h"
 
void test_AdminManager::initTestCase() {
  auto db = QSqlDatabase::addDatabase("QSQLITE");
  db.setDatabaseName("H:/QtProject/LazyMins/a.db");
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

void test_AdminManager::testCreate()
{
    AdminManager am;
    
    QList<QSqlRecord> rr;
    for(int n=0; n < users.count() - 1; ++n) {
      auto oRec = am.create(users[n]);
      QCOMPARE(oRec.has_value(), true);
      if (oRec) {
        rr << *oRec;
      }
    }
    QCOMPARE(rr.count(), 3);
    
    // user3 [madun] must be fail
    auto optFail = am.create(users[3]);
    
    QCOMPARE(optFail.has_value(), false);
    
    QCOMPARE(am.exists(1),  true);
    QCOMPARE(am.exists(2),  true);
    QCOMPARE(am.exists(3),  true);
    QCOMPARE(am.exists(4), false);
    
    QCOMPARE(am.exists("nurholis"), true );
    QCOMPARE(am.exists("maman"),    true );
    QCOMPARE(am.exists("syahid"),   true );
    QCOMPARE(am.exists("madun"),    false);
    QCOMPARE(am.exists("maroon"),   false);
    
    // change password
    QCOMPARE(am.changePassword("maman", "maerohewag"), true);
    QCOMPARE(am.changePassword("syahid", "fusuydihays"), true);
    
    auto &auth = AuthManager::instance();
    
    QCOMPARE(auth.passwordMatch("maman", "maerohewag"), true);
    QCOMPARE(auth.passwordMatch("syahid", "fusuydihays"), true);
    
    QCOMPARE(am.remove(rr[0].value("id").toInt()), true);
    QCOMPARE(am.remove(rr[1].value("id").toInt()), true);
    QCOMPARE(am.remove(rr[2].value("id").toInt()), true);
    
    QCOMPARE(am.remove(5), false);
    QCOMPARE(am.remove(6), false);
    QCOMPARE(am.remove(7), false);
}

QTEST_MAIN(test_AdminManager);
