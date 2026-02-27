#pragma once

#include "formdialog.h"

namespace Ui {
  class ProductDialog;
}

class ProductDialog : public FormDialog
{
  Q_OBJECT
  public:
    ProductDialog(QWidget *p=nullptr);
    ~ProductDialog();
  
  protected:
    void setupFields() override;
    void setupBoundFields() override;
    bool onSave(const QVariantMap& m) override;
  
  private:
    Ui::ProductDialog *ui;
};