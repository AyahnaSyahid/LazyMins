#pragma once

#include <QDialog>

namespace Ui {
    class OrderPickerDialog;
}

#include <QSqlQueryModel>

class OrderPickerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OrderPickerDialog(QWidget *parent = nullptr);
    ~OrderPickerDialog();

    void setCustomerId(int cid);
    void setFilterIds(const QList<int> ids);
    
private slots:
    void on_orderView_clicked(const QModelIndex &index);
    void onParameterChanged();

signals:
    void ordersPicked(const QList<int> oids);
    void parameterChanged();
    
private:
    Ui::OrderPickerDialog *ui;
    QSqlQueryModel *model;
    int m_customer_id = -1;
    QList<int> m_filter_ids;
};