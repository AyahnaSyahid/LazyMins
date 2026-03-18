#pragma once

#include "formdialog.h"
#include "src/managers/adminmanager.h"

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
    QString getErrorString() const { return m_adminManager.errorString(); };

protected:
    bool onSave(const QVariantMap &changes) override;
    void setupFields() override;
    bool validateFields(const QVariantMap &changes);

private slots:
    void on_simpanButton_clicked();

private:
    Ui::UserDialog *ui;
    AdminManager m_adminManager;
};
