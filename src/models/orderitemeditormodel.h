#pragma once

#include <QAbstractTableModel>
#include <QVariant>
#include <QHash>
#include <QSqlRecord>
#include <QSqlTableModel>
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
        Col_SizeWidth,
        Col_SizeHeight,
        Col_UseArea,
        Col_SalePrice,
        Col_BasePrice,
        Col_DiscountPercentage,
        Col_DiscountAmount,
        Col_Subtotal,
        Col_Total,
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

    bool appendRow(const QVariantMap &defaultValues = QVariantMap());
    bool removeRow(int row, const QModelIndex &parent = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    void revertAllChanges();
    bool isDirty() const;

    // kalkulasi subtotal
    double calculateSubtotal() const;
    int orderItemIdForRow(int r) const;
    
    struct ModelSaveResult {
      bool ok = false;
      QString error;
    };
    
    ModelSaveResult saveModel(const QSqlRecord& rec);
    
signals:
    void subtotalChanged();

private:
    static QString columnKey(int column);
    int m_orderid;
    QList<QSqlRecord>               m_fromDatabase;
    QHash<QPair<int,int>, QVariant> m_editedCells;
    QList<QVariantMap>              m_newData;
    OrderItemManager                oim;
    StockConsumesRepo               scr;
    // untuk lookup flags width dan height apakah bisa diedit atau tidak
    QSqlTableModel *m_tableModel = nullptr;
};