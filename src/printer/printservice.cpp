#include "printservice.h"
#include "posprinter.h"

PrintService::PrintService() : QObject(nullptr)
{
  qRegisterMetaType(ReceiptPtr);
}
