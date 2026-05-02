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
    enum SetMode {
        SetById,
        SetByName,
        NotSet
    };

    explicit OrderPickerDialog(QWidget *parent = nullptr);
    ~OrderPickerDialog();

    void setCustomerId(int cid);
    void setCustomerName(const QString& name);
    void setFilterIds(const QList<int>& ids);
    int  availableCount() const { return model->rowCount(); }
    
    SetMode setBy() const { return m_setMode; }

private slots:
    void on_orderView_clicked(const QModelIndex &index);
    void onParameterChanged();

signals:
    void ordersPicked(const QList<int> oids);
    void parameterChanged();
    
private:
    SetMode m_setMode = NotSet;
    QString m_customerName;
    Ui::OrderPickerDialog *ui;
    QSqlQueryModel *model;
    int m_customer_id = -1;
    QList<int> m_filter_ids;
};