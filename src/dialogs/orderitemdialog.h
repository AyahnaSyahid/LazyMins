#pragma once

#include "formdialog.h"
#include "src/managers/managers.h"
namespace Ui {
    class OrderItemDialog;
}

class OrderItemDialog : public FormDialog {
    Q_OBJECT

public:
    explicit OrderItemDialog(QWidget *parent = nullptr);
    ~OrderItemDialog();
    
    bool onSave(const QVariantMap& changes) override;
    void setupFields() override;
    void setupBoundFields() override;

    QVariantMap getFieldData() const {
        return collect();
    }
    
    void setAutoCommit(bool autoCommit) {
        m_autoCommit = autoCommit;
    }
    
    void setCustomerPriceLevel(int priceLevel) {
        // kita handle ini di implementasi on_produkComboBox_currentIndexChanged, jadi simpan saja di member variable
        m_customerPriceLevel = priceLevel;
    }

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

signals:
    void editFinished(const QVariantMap& itemData);

private:
    double calculatedPrice() const;
    Ui::OrderItemDialog *ui;
    bool m_autoCommit = true;
    int m_customerPriceLevel = 1;
    ProductPriceManager m_priceManager;
};