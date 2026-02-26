#pragma once

namespace Ui {
  class KonsumenDialog;
}

#include "src/dialogs/formdialog.h"

class KonsumenDialog : public FormDialog
{
  Q_OBJECT
  public:
    KonsumenDialog(QWidget *p = nullptr);
  
  protected:
    void setupFields() override;
    bool onSave(const QVariantMap& data) override;
  
  private slots:
    void on_simpanButton_clicked();
  
  private:
    Ui::KonsumenDialog *ui;
};