#ifndef INVOICEVIEWS_H
#define INVOICEVIEWS_H

#include <QWidget>

namespace Ui {
  class InvoiceViews;
};

class InvoiceViews : public QWidget
{
  Q_OBJECT
  public:
    explicit InvoiceViews(QWidget *parent=nullptr);
    ~InvoiceViews();
  
  private slots:
    void on_view_customContextMenuRequested(const QPoint &p);
    
  
  private:
    Ui::InvoiceViews *ui;
    QSqlQueryModel *queryModel;
};

#endif