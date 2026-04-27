#pragma once
#include "basemanager.h"

class OrderItemFinishingManager : public BaseManager {
 public:
  explicit OrderItemFinishingManager();

  QList<QSqlRecord> getByOrderItem(int orderItemId);
  void setOrderItem(int id, int orderItemId);
};
