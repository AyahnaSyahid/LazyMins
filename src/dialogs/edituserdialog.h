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
    void userNotFound();
    
  signals:
    void userdataChanged(const QString& username);
  
  private:
    struct UserInfo {
      int id, role_id;
      QString username,
              nama_lengkap,
              email,
              nomor_telp;
    };
    
    Ui::EditUserDialog *ui;
    UserInfo info;
};