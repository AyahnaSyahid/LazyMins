#pragma once

#include <QDialog>

namespace Ui {
  class PencatatPengeluaran;
}

class PencatatPengeluaran : public QDialog
{
  Q_OBJECT
  public:
    PencatatPengeluaran(QWidget *p);
    ~PencatatPengeluaran();
  
  private slots:
    void on_simpanButton_clicked();

  private:
    Ui::PencatatPengeluaran *ui;
    QString m_admin, m_tipe, m_detail;
    qlonglong m_amount;
};