#include "productcategoriesmanager.h"

bool ProductCategoriesManager::setState(int id, int state) {
  return update(id, {{"is_active", state ? true : false}});
}