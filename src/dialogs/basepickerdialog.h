#pragma once

#include <QDialog>
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QSqlError>

namespace Ui {
    class BasePickerDialog;
}

class BasePickerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BasePickerDialog(const QString &title, QWidget *parent = nullptr);
    virtual ~BasePickerDialog();

    // Mengembalikan false jika query gagal
    bool setupData(const QString &query, const QMap<int, QString> &headers);
    void setHiddenColumns(const QList<int> &columns);

    // Fungsi untuk menyesuaikan ukuran dialog dengan konten
    void adjustDialogSize();

signals:
    void idPicked(int id);

protected slots:
    virtual void onItemSelected(const QModelIndex &index);
    void onFilterTimerTimeout();

protected:
    Ui::BasePickerDialog *ui;
    QSqlQueryModel *m_queryModel;
    QSortFilterProxyModel *m_proxyModel;
    QTimer *m_filterTimer;

    int getIdFromIndex(const QModelIndex &index);
};