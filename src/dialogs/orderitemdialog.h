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

private:
    Ui::OrderItemDialog *ui;
};