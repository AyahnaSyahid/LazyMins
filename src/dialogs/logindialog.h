#pragma once

#include <QDialog>

namespace Ui {
  class LoginDialog;
}

class LoginDialog : public QDialog
{
  Q_OBJECT

public:
  explicit LoginDialog(QWidget *p=nullptr);
  ~LoginDialog();

public slots:
  void setUsername(const QString& name);

private slots:
  void on_masukButton_clicked();
  void decrementChances();
  void reenableLogin();

private:
  int m_failCount;
  Ui::LoginDialog *ui;
};