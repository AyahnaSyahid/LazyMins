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
    void updateQuery();

private:
    void setupHeaderData();
    void setupItemDelegate();
    void setupFilterConnection();
    void setupDateEdit();
    void finalizeUi();
    void initializeMinMax(int KID);
    QList<QDate> minMax;
    int m_customerId{0};

    QString buildQuery() const;

    Ui::KonsumenOrderBrowser *ui;
    QSqlQueryModel *model;
    QSortFilterProxyModel *proxy;
};