#pragma once

#include <QDialog>

namespace Ui {
  class EditUserLoginInfoDialog;
}

class EditUserLoginInfoDialog : public QDialog
{
  Q_OBJECT
  public:
    EditUserLoginInfoDialog(const QString& username, QWidget *p=nullptr);
    ~EditUserLoginInfoDialog();
    const QString& currentUsername() const { return m_username; }

  private slots:
    void on_simpanButton_clicked();
  
  private:
    Ui::EditUserLoginInfoDialog *ui;
    QString m_username;
};