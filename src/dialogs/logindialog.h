#pragma once

#include <QDialog>

namespace Ui {
  class LoginDialog;
}


class QCloseEvent;
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

protected:
  void closeEvent(QCloseEvent* evt) override;
  void confirmAndExit();
  void reject() override;

private:
  int m_failCount;
  bool m_confirmingExit = false;
  Ui::LoginDialog *ui;
};
