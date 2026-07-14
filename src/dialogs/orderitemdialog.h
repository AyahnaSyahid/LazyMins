#pragma once

#include "src/models/finishinglistmodel.h"
#include "src/managers/managers.h"

namespace Ui {
    class OrderItemDialog;
}

#include <QDialog>

class OrderItemDialog : public QDialog {
    Q_OBJECT

public:
    enum Mode {
      Create,
      Modify
    };

    struct OrderItemEditResult {
        OrderItem item;
        int rowNumber;
        OrderItemEditResult(OrderItem o, int r) : item(o), rowNumber(r) {}
    };

    explicit OrderItemDialog(QWidget *parent = nullptr);
    ~OrderItemDialog();

    void setCustomerPriceLevel(int priceLevel) {
        // kita handle ini di implementasi on_produkComboBox_currentIndexChanged, jadi simpan saja di member variable
        m_customerPriceLevel = priceLevel;
    }
    
    void setOrder(OrderItem order, int row=-1);

    OrderItemEditResult editResult() const;

    const FinishingListModel& finishingModel() const { return m_finishingListModel; }
    const Mode &mode() { return m_mode; }


public slots:
    void resetForm();

private slots:
    void setupProdukComboBox();
    void setupFinishingView();
    OrderItem buildOrderItemFromUi(const QSqlRecord& record) const;

    void on_produkComboBox_currentIndexChanged(int index);
    void on_simpanButton_clicked();
    void on_pilihButton_clicked();

    void on_hargaSpinBox_valueChanged(int arg1);
    void on_qtySpinBox_valueChanged(int arg1);
    void on_diskonDoubleSpinBox_valueChanged(double arg1);
    void on_diskonRpSpinBox_valueChanged(int arg1);
    void on_widthBox_valueChanged(double arg1) { recalculateSubtotal(); }
    void on_heightBox_valueChanged(double arg1) { recalculateSubtotal(); }
    void recalculateSubtotal();
    void setCurrentProduct(int);
    
    void on_tambahButton_clicked();

    // handle createFinishing
    void onCreateFinishing(const FinishingItem& item);
    
    // handle editFinishing
    void onFinishingEdited(const FinishingItem& item);
    
    void on_finishingView_customContextMenuRequested(const QPoint& p);
    
signals:
    void itemCreated(const OrderItem& orderItem);
    void editFinished();

private:
    int calculatedPrice() const;
    Ui::OrderItemDialog *ui;
    int m_customerPriceLevel = 1;
    FinishingListModel m_finishingListModel;
    QList<FinishingItem> m_newFinishingItems;
    ProductPriceManager m_priceManager;
    ProductManager m_productManager;
    Mode m_mode;
    OrderItem m_itemEdit;
    int m_rowEdit;
};