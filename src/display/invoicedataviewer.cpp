#include "invoicedataviewer.h"
#include "src/display/ui_dataviewer.h"

InvoiceDataViewer::InvoiceDataViewer(QWidget *p):
DataViewer(p)
{
  ui = DataViewer::Ui();
  setQueryArgs(R"-(
    
  )-");
}

InvoiceDataViewer::~InvoiceDataViewer() {}
