#ifndef InvoicesManagerModel_H
#define InvoicesManagerModel_H

#include <QSortFilterProxyModel>

class Database;
class QModelIndex;
class InvoicesManagerModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit InvoicesManagerModel(Database*, QObject* =nullptr);
    ~InvoicesManagerModel();

    QVariant data(const QModelIndex&, int =Qt::DisplayRole) const;
    // QVariant headerData(int, Qt::Orientation, int =Qt::DisplayRole) const;

public slots:
    void showPaidInvoices(bool);
    void select();

private:
    using QSortFilterProxyModel::setSourceModel;
    Database* db;

};

#endif
