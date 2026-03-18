#pragma once

#include <QDialog>

namespace Ui {
  class CreateUserDialog;
}

class QSqlQueryModel;
class CreateUserDialog : public QDialog
{
  Q_OBJECT
  public:
    explicit CreateUserDialog(QWidget *p=nullptr);
    ~CreateUserDialog();
  
  private slots:
    void on_simpanButton_clicked();
    

  private:
    bool checkInputs();

  signals:
    void userAdded();
  
  private:
    Ui::CreateUserDialog *ui;
    QSqlQueryModel * qmodel;
};