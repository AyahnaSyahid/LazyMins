#pragma once

#include <QDialog>

namespace Ui {
  class CreateFinishingServiceDialog;
}

class CreateFinishingServiceDialog : public QDialog
{
  Q_OBJECT
  public:
    explicit CreateFinishingServiceDialog(QWidget * = nullptr);
    ~CreateFinishingServiceDialog();
  
  private slots:
    void on_simpanButton_clicked();
    
  private:
    Ui::CreateFinishingServiceDialog* ui;
};