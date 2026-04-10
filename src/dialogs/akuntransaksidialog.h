#pragma once

#include "formdialog.h"

namespace Ui {
  class AkunTransaksiDialog;
}

class AkunTransaksiDialog : public FormDialog
{
  Q_OBJECT
  public:
    explicit AkunTransaksiDialog(QWidget * = nullptr);
    ~AkunTransaksiDialog();
    
    bool isInputAcceptable() const override;
    
  protected:
    void setupFields() override;
    void setupBoundFields() override;
    bool onSave(const QVariantMap& vals) override;
 
  private slots:
    void on_simpanButton_clicked();

  private:
    Ui::AkunTransaksiDialog *ui;
};