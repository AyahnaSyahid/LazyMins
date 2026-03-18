#pragma once

#include "formdialog.h"
#include "src/managers/managers.h"
#include "src/models/finishinglistmodel.h"

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

    explicit OrderItemDialog(QWidget *parent = nullptr);
    ~OrderItemDialog();

    void setCustomerPriceLevel(int priceLevel) {
        // kita handle ini di implementasi on_produkComboBox_currentIndexChanged, jadi simpan saja di member variable
        m_customerPriceLevel = priceLevel;
    }
    
    void setOrder(OrderItem *order);
    
    const FinishingListModel& finishingModel() const { return m_finModel; }
    const Mode &mode() { return m_mode; }

public slots:
    void resetForm();

private slots:
    void on_produkComboBox_currentIndexChanged(int index);
    void on_simpanButton_clicked();

    void on_hargaSpinBox_valueChanged(int arg1);
    void on_qtySpinBox_valueChanged(int arg1);
    void on_diskonDoubleSpinBox_valueChanged(double arg1);
    void on_diskonRpSpinBox_valueChanged(int arg1);
    void on_widthBox_valueChanged(double arg1) { recalculateSubtotal(); }
    void on_heightBox_valueChanged(double arg1) { recalculateSubtotal(); }
    void recalculateSubtotal();
    
    void on_tambahButton_clicked();

    // handle createFinishing
    void onCreateFinishing(const FinishingItem& item);
    
    // handle editFinishing
    void onFinishingAccepted();
    
signals:
    void editFinished(const OrderItem& orderItem);

private:
    int calculatedPrice() const;
    Ui::OrderItemDialog *ui;
    int m_customerPriceLevel = 1;
    FinishingListModel m_finModel;
    ProductPriceManager m_priceManager;
    ProductManager m_productManager;
    Mode m_mode;
    OrderItem *m_orderItem;
};