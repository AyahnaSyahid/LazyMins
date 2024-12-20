#ifndef ConfigH
#define ConfigH
#include <QtDebug>

#ifndef QAPPNAME
#define QAPPNAME "LazyMin"
#endif
#ifndef QAPPORG
#define QAPPORG "LazyOrg"
#endif

namespace GlobalNamespace {
    extern qint64 logged_user_id;
}

namespace DB_INIT {
  bool initializeDatabase();
  void fillDummyDatabaseData();
}

#endif // ConfigH