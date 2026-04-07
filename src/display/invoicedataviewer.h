#pragma once

#include "src/display/dataviewer.h"
#include "src/display/ui_dataviewer.h"

class InvoiceDataViewer : public DataViewer
{
  Q_OBJECT
  public:
    InvoiceDataViewer(QWidget * = nullptr);
    ~InvoiceDataViewer();
  
  private:
    Ui::InvoiceDataViewer *ui;
};