#pragma once

#include <QAbstractTableModel>
#include <QVariant>
#include <QHash>
#include <QSqlRecord>
#include "src/managers/managers.h"

class OrderItemEditorModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        Col_Id = 0,
        Col_OrderId,
        Col_ProductId,
        Col_ProductName,
        Col_Sku,
        Col_Quantity,
        Col_Unit,
        Col_BasePrice,
        Col_DiscountPercentage,
        Col_DiscountAmount,
        Col_Subtotal,
        Col_Notes,
        Col_CreatedAt,
        Col_UpdatedAt,
        Col_COUNT
    };

    explicit OrderItemEditorModel(QObject *p = nullptr);
    ~OrderItemEditorModel();

    bool loadFromOrder(int orderid);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& ix, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& ix, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex& mi) const override;

    QList<QVariantMap> pendingNewRows() const { return m_newData; }
    QHash<QPair<int,int>, QVariant> editedCells() const { return m_editedCells; }

private:
    static QString columnKey(int column);

    QList<QSqlRecord>               m_fromDatabase;
    QHash<QPair<int,int>, QVariant> m_editedCells;
    QList<QVariantMap>              m_newData;
    OrderItemManager                oim;
};