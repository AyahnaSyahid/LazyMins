#pragma once

#include <QAbstractListModel>

struct SavedOrder {
  int id = -1;
  QString order_number;
  int subtotal = 0;
  int discount_amount = 0;
  int total_amount = 0;
  
  void validate();
  SavedOrder &loadFromId(int order_id);
};

class InvoiceComposerModel : public QAbstractListModel
{
  Q_OBJECT
  public:
    enum Role {
      IdRole       = Qt::UserRole + 1, // internal use dont show
      NumberRole,     // Show as Bold Text
      SubtotalRole,   
      DiscountRole,
      DateRole,       // unimplemented
      TotalRole       // at bottom right of delegate
    };

    InvoiceComposerModel(QObject * =nullptr);
    ~InvoiceComposerModel();
    
    bool     loadInvoice(int invoiceId);

    int      rowCount(const QModelIndex& = QModelIndex()) const override;
    QVariant data(const QModelIndex& ix, int role) const override;
    
    void     insertOrder(int order_id);
    void     removeOrder(int order_id);
    
    QList<int> imported() const;

  private:
    int m_invoice_id = -1;
    
    QList<SavedOrder> m_orders;
};