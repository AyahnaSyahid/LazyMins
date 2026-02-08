#ifndef PRODUCTCATEGORIESMANAGER_H
#define PRODUCTCATEGORIESMANAGER_H

#include "basemanager.h"

class ProductCategoriesManager : public BaseManager {
  public:
    ProductCategoriesManager() : BaseManager("product_categories", false) {}
    ~ProductCategoriesManager() {}
    
    bool setState(int id, int state);
};

#endif