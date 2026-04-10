#pragma once

#include "basemanager.h"

class KonsumenManager : public BaseManager
{
  public:
    KonsumenManager() : BaseManager("konsumen", false) {}
    ~KonsumenManager() {}
    int getPriceLevelById(int) const;
  
  private:
    void beforeUpdate(int id, QVariantMap &m) override;
    void beforeCreate(QVariantMap &m) override;
};
