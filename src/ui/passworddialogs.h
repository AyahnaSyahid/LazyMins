#ifndef PasswordDialogs_H
#define PasswordDialogs_H

#include <QDialog>

namespace Ui {
  class ChangePasswordDialog;
  class RevokePasswordDialog;
  class UserSelectorDialog;
};

class UserManager;
class ChangePasswordDialog : public QDialog {
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

// ==============================================

class ChangeOtherPasswordDialog: public ChangePasswordDialog {
  Q_OBJECT

public:
  explicit ChangeOtherPasswordDialog(int userId, UserManager* uman, QWidget* =nullptr);
  ~ChangeOtherPasswordDialog();

private slots:
  void revokePassword();

private:
  int euid;
};

// ==============================================

class RevokePasswordDialog : public QDialog {
  Q_OBJECT

public:
  explicit RevokePasswordDialog(UserManager*, QWidget* = nullptr);
  ~RevokePasswordDialog();

private slots:
  void on_okButton_clicked();

private
  Ui::RevokePasswordDialog *ui;
  UserManager *uman;

signals:
  void revokeSuccess();

};

// ==============================================

class UserSelectorDialog : public QDialog {
  Q_OBJECT

public:
  explicit UserSelectorDialog(QWidget *parent);
  ~UserSelectorDialog();
  const int& selectedId() const { return sid; };

private slots:
  void on_pilihButton_clicked();

private:
  int sid;
};
#endif