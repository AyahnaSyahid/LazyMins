#ifndef PasswordDialogs_H
#define PasswordDialogs_H

#include <QDialog>

namespace Ui {
  class ChangePasswordDialog;
}

class UserManager;
class ChangePasswordDialog(QDialog) {
  Q_OBJECT

public:
  explicit ChangePasswordDialog(UserManager* uman, QWidget* parent=nullptr);
  ~ChangePasswordDialog();

private slots:
  void on_saveButton_clicked();

private:
  Ui::ChangePasswordDialog *ui;
  UserManager *uman;
};

// class ChangeOtherPasswordDialog(QDialog)
#endif