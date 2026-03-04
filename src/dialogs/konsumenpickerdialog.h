#pragma once

#include <QDialog>

namespace Ui {
    class KonsumenPickerDialog;
}

#include <QSqlQueryModel>

class KonsumenPickerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit KonsumenPickerDialog(QWidget *parent = nullptr);
    ~KonsumenPickerDialog();

private slots:
    void on_konsumenView_clicked(const QModelIndex &index);

private:
    Ui::KonsumenPickerDialog *ui;
    QSqlQueryModel *model;

};