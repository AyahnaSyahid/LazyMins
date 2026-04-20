#pragma once

#include <QDialog>
#include <QSqlRecord>

namespace Ui {
  class AkunTransaksiOpnameDialog;
}

class AkunTransaksiOpnameDialog : public QDialog
{
  Q_OBJECT
  public:
    explicit AkunTransaksiOpnameDialog(QWidget *parent = nullptr);
    ~AkunTransaksiOpnameDialog();
    bool prepareOpname(int akunId);

  private slots:
    void on_simpanButton_clicked();
    void on_realBox_valueChanged(int);

  private:
    Ui::AkunTransaksiOpnameDialog *ui;
    QSqlRecord m_record;
};