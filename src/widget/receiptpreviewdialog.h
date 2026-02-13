#ifndef RECEIPTPREVIEWDIALOG_H
#define RECEIPTPREVIEWDIALOG_H

#include <QDialog>
#include <QGraphicsView>
#include <QGraphicsScene>
#include "../invoicedatatype.h"

namespace Ui { class ReceiptPreviewDialog; }

class ReceiptPreviewDialog : public QDialog
{
  Q_OBJECT
  public:
    explicit ReceiptPreviewDialog(qlonglong inv_id, QWidget *parent=nullptr);
    ~ReceiptPreviewDialog();
    bool isReady() const { return m_ready; }
    
  
  public slots:
    void draw();
  
  private:
    Ui::ReceiptPreviewDialog *ui;
    QGraphicsScene *scene;
    bool m_ready;
    PrintInvoiceParams param;
};

#endif // RECEIPTPREVIEWDIALOG_H