#ifndef EDITINVOICEDIALOG_H
#define EDITINVOICEDIALOG_H

namespace Ui {
  class CreateInvoiceDialog;
}
#include "createinvoicedialog.h"
#include "../invoicedatatype.h"

class EditInvoiceDialog : public CreateInvoiceDialog
{
  Q_OBJECT
  public:
    explicit EditInvoiceDialog(int invoice_id, QWidget *p=nullptr);
    ~EditInvoiceDialog();

  protected slots:
    void on_simpanButton_clicked() override;  

  private:
    InvoiceData m_old;
    int targetInvoice;
};

#endif