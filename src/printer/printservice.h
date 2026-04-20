#pragma once

#include "receipt.h"
#include <QObject>

class PrintService : public QObject
{
  Q_OBJECT
  public:
    static PrintService& instance();
  
  public slots:
    void printReceipt(const ReceiptPtr &ptr);
  
  private:
    PrintService();
    PrintService(PrintService&&) = delete;
};