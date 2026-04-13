#pragma once

#include "basemanager.h"

class KonsumenManager : public BaseManager
{
  public:
    KonsumenManager() : BaseManager("konsumen", false) {}
    ~KonsumenManager() {}
    int getPriceLevelById(int) const;
  
  protected:
    QVariantMap validateParams(const QVariantMap& params) override;
  
  private:
    bool beforeUpdate(int id, QVariantMap &m) override;
    bool beforeCreate(QVariantMap &m) override;
};
