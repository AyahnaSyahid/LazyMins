#ifndef PRODUCTPRICESMANAGER_H
#define PRODUCTPRICESMANAGER_H

#include "basemanager.h"

class ProductPricesManager : public BaseManager {
  public:
    ProductPricesManager() : BaseManager("product_prices", false) {}
    ~ProductPricesManager() {}
  
    bool exists(int pid, int levid) const;
    bool setPrice(int pid, int levid, int newprice);
};

#endif