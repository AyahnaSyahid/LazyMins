#pragma once

#include "basemanager.h"

class ProductCategoriesManager : public BaseManager
{
  public:
    ProductCategoriesManager() : BaseManager("product_categories", false) {};
    ~ProductCategoriesManager() {}
};
