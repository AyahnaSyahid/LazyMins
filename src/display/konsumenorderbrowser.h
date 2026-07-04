#pragma once

#include <QDialog>

namespace Ui
{
    class KonsumenOrderBrowser;
}

class QSqlQueryModel;
class QSortFilterProxyModel;
class KonsumenOrderBrowser : public QDialog
{
    Q_OBJECT
public:
    KonsumenOrderBrowser(QWidget *parent = nullptr);
    ~KonsumenOrderBrowser();
    bool setCustomerId(int KID);

private slots:

private:
    void setupHeaderData();
    void setupItemDelegate();
    void setupFilterConnection();
    void setupDateEdit();
    void finalizeUi();
    QList<QDate> minMax;

    QString buildQuery() const;

    Ui::KonsumenOrderBrowser *ui;
    QSqlQueryModel *model;
    QSortFilterProxyModel *proxy;
};