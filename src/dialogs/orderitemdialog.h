#pragma once

#include "formdialog.h"

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

private slots:
    void on_simpanButton_clicked() { onSave(collect()); }
private:
    Ui::OrderItemDialog *ui;
};