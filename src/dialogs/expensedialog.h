#pragma once

namespace Ui {
    class ExpenseDialog;
}

#include <QDialog>
#include <QVariantMap>

class ExpenseDialog : public QDialog
{
  Q_OBJECT
  public:
    explicit ExpenseDialog(QWidget * =nullptr);
    ~ExpenseDialog();

    QVariantMap collectParams() const;
    
  private slots:
    void on_simpanButton_clicked();
    void on_pilihKategori_clicked();
    void on_pilihAkun_clicked();
  
  signals:
    void expenseAdded();

  private:
    Ui::ExpenseDialog *ui;
};