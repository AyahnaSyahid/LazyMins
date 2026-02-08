#ifndef KONSUMENMANAGER_H
#define KONSUMENMANAGER_H

#include "basemanager.h"
class KonsumenManager : public BaseManager
{
  public:
    KonsumenManager() : BaseManager("konsumen", false) {}
    ~KonsumenManager() {}
    
    QSqlRecord create(const QVariantMap& map) override;
};
#endif