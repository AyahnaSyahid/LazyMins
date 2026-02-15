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
  
  private:
    InvoiceData m_old;
};

#endif