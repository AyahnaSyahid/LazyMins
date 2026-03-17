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
  enum Mode {
    Create,
    Modify
  };

  explicit FinishingDialog(QWidget *parent = nullptr);
  ~FinishingDialog();
  
  void setItem(FinishingItem *item);

  const Mode &mode() const { return m_mode; }
  
private slots:
  void recalculate();
  void on_simpanButton_clicked();

signals:
  void createItem(const FinishingItem& fi);
  
private:
  Ui::FinishingDialog *ui;
  QSqlQueryModel *m_finishingModel;
  QTableView *m_finishingView;
  FinishingItem *m_item;
  Mode m_mode;
};