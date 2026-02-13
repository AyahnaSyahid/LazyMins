#ifndef INVOICEVIEWS_H
#define INVOICEVIEWS_H

#include <QWidget>

namespace Ui {
  class InvoiceViews;
};

class QSqlQueryModel;
class QSortFilterProxyModel;
class InvoiceViews : public QWidget
{
  Q_OBJECT
  public:
    explicit InvoiceViews(QWidget *parent=nullptr);
    ~InvoiceViews();
  
    QString modelQuery(bool viewLunas=false) const;
    
  private slots:
    void showPreview();
    void on_view_customContextMenuRequested(const QPoint &p);
    void on_checkBox_toggled(bool);
    
  
  private:
    Ui::InvoiceViews *ui;
    QSqlQueryModel *queryModel;
    QSortFilterProxyModel *proxy;
    
};

#endif