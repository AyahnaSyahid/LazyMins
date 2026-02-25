#pragma once

#include <QDialog>

namespace Ui {
  class EditUserDialog;
}

class EditUserDialog : public QDialog
{
  Q_OBJECT
  
  public:
    EditUserDialog(const QString& username, QWidget *p=nullptr);
    ~EditUserDialog();
  
  private slots:
    void on_ubahButton_clicked();
    void on_simpanButton_clicked();
  
  signals:
    void userdataChanged(const QString& username);
  
  private:
    Ui::EditUserDialog *ui;
};