#pragma once

#include "src/display/dataviewer.h"

namespace Ui {
  class DataViewer;
}

class InvoiceDataViewer : public DataViewer
{
  Q_OBJECT
  public:
    InvoiceDataViewer(QWidget * = nullptr);
    ~InvoiceDataViewer();
  
  private:
    Ui::DataViewer *ui;
};