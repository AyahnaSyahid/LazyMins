#pragma once

#include <QDialog>

namespace Ui {
  class RevokePasswordDialog;
}

class RevokePasswordDialog : public QDialog
{
  Q_OBJECT
  public:
    RevokePasswordDialog(QWidget *p=nullptr);
    ~RevokePasswordDialog();
    
  private slots:
    void on_konfirmasiButton_clicked();
  private:
    Ui::RevokePasswordDialog *ui;
};
