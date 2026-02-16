#ifndef INVOICEVIEWS_H
#define INVOICEVIEWS_H

#include "realtimedatawidget.h"

namespace Ui {
  class InvoiceViews;
};

class QSqlQueryModel;
class QSortFilterProxyModel;
class InvoiceViews : public RealTimeDataWidget
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
    void reloadModelData(const QList<QString> &tables) override;
  
  signals:
    void editInvoiceRequest(int inv_id) const;
    void repaymentRequest(int inv_id) const;
  
  private:
    Ui::InvoiceViews *ui;
    QSqlQueryModel *queryModel;
    QSortFilterProxyModel *proxy;
};

#endif