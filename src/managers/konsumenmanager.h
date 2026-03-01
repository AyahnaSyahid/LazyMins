#pragma once

#include "basemanager.h"

class KonsumenManager : public BaseManager
{
  public:
    KonsumenManager() : BaseManager("konsumen", false) {}
    ~KonsumenManager() {}
  
  private:
    void beforeUpdate(int id, QVariantMap &m) override;
    void beforeCreate(QVariantMap &m) override;
};
