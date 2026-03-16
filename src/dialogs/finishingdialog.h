#pragma once

namespace Ui{
    class FinishingDialog;
}

#include <QDialog>
#include "src/models/ordermodel.h"

class QSqlQueryModel;
class QTableView;
class FinishingDialog : public QDialog
{
  Q_OBJECT
public:
  explicit FinishingDialog(QWidget *parent = nullptr);
  ~FinishingDialog();

private slots:
  void recalculate();
  
private:
  Ui::FinishingDialog *ui;
  QSqlQueryModel *m_finishingModel;
  QTableView *m_finishingView;
};