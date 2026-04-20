#pragma once

namespace Ui {
    class DepositDialog;
}

#include <QDialog>
#include <QSqlRecord>

class DepositDialog : public QDialog
{
    Q_OBJECT
    public:
        explicit DepositDialog(QWidget *parent = nullptr);
        ~DepositDialog();    
        void prepareModify(const QSqlRecord& rec);
    private slots:
        void on_simpanButton_clicked();
    private:
        Ui::DepositDialog *ui;
        QSqlRecord m_record;
};