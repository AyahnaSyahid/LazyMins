#pragma once

#include <QDialog>

namespace Ui {
    class ProductPickerDialog;
}

class ProductPickerDialog : public QDialog
{
  Q_OBJECT
  
  public:
    explicit ProductPickerDialog(QWidget * = nullptr);
    ~ProductPickerDialog();
  
  private slots:
    void on_productView_clicked(const QModelIndex& ix);
    void on_productView_activated(const QModelIndex& ix);
    void onFilterTimerTimeout();
  
  signals:
    void productPicked(int product_id);
  
  private:
    Ui::ProductPickerDialog *ui;
};