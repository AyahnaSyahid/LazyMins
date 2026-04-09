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

  private slots:
    
  private:
    Ui::DataViewer *ui;
    QAction *m_createInvoiceAction;
    
};