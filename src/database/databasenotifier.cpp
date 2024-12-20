#include "databasenotifier.h"
DatabaseNotifier* DatabaseNotifier::_inst = nullptr;
QMutex DatabaseNotifier::mutex;