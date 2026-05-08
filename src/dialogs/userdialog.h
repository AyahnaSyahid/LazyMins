#pragma once

#include "formdialog.h"

namespace Ui
{
    class UserDialog;
}

class UserDialog : public FormDialog
{
    Q_OBJECT
public:
    explicit UserDialog(QWidget *parent = nullptr);
    ~UserDialog();

protected:
    bool onSave(const QVariantMap &changes) override;
    void setupFields() override;
    void setupBoundFields() override;
    bool isInputAcceptable() const override;

private slots:
    void on_simpanButton_clicked();

private:
    Ui::UserDialog *ui;
};
