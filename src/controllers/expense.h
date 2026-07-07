#pragma once

#include <QString>

class ExpenseController {
 public:
  ExpenseController() = default;
  ~ExpenseController() = default;

  bool recordExpense(int akun_id, qint64 jumlah, const QString& keterangan, int *recordId, QString *error);
};