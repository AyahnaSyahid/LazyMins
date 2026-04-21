#pragma once

#include "src/display/dataviewer.h"

namespace Ui {
  class DataViewer;
}

class QAction;
class InvoiceDataViewer : public DataViewer
{
  Q_OBJECT
  
  public:
    InvoiceDataViewer(QWidget * = nullptr);
    ~InvoiceDataViewer();
    QAction *createInvoiceAction() { return m_createInvoiceAction; }

  public slots:
    void onCreateInvoice();
    void openContextMenu(const QPoint& pt);
    void on_dataView_customContextMenuRequested(const QPoint& p);
    void openPaymentForInvoice(int);
    
  signals:
    // penerusan signal
    void invoiceCreated(int id);
    void paymentCreated(int id);
    void printInvoiceToSerial(int id);
  
  private slots:
    void onBrowseInvoices();
    
  private:
    void onPaymentGranted(const QVariantMap& vm);
  
    Ui::DataViewer *ui;
    QAction *m_createInvoiceAction;
    
    QMenu* dataBaruMenu;
};