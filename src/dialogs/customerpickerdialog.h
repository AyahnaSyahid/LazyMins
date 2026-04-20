#pragma once

#include <QDialog>

namespace Ui {
    class CustomerPickerDialog;
}

#include <QSqlQueryModel>

class CustomerPickerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CustomerPickerDialog(QWidget *parent = nullptr);
    ~CustomerPickerDialog();

    int availableCustomers() const { return model->rowCount(); }
    
    void setModelQuery(const QString& q);
    
private slots:
    void on_customerView_clicked(const QModelIndex &index);

signals:
    void customerPicked(const QSqlRecord& record);
    
private:
    Ui::CustomerPickerDialog *ui;
    QSqlQueryModel *model;
};