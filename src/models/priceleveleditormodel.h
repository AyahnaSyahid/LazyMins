#pragma once

#include <QAbstractTableModel>
#include <QMap>
#include <QList>
#include <QVariant>
#include "src/managers/managers.h"

class PriceLevelEditorModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum CustomRole {
        IdRole = Qt::UserRole + 1,
        LevelNameRole,
        DescriptionRole,
        DiscountRole,
        DefaultPriceRole,
        IsNewRole
    };

    explicit PriceLevelEditorModel(QObject *parent = nullptr);

    // Konfigurasi Produk
    void setProductId(int pid);

    // Table Model Overrides
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Actions
    void loadLevels();
    void addNewLevel();
    bool commit(); // Fungsi untuk simpan ke database via Managers
    bool isDirty() const;

private:
    struct PriceLevel {
        int id = -1;
        QString level_name;
        QString description;
        double discount_percentage = 0.0;
        bool isNew = false;
    };

    int m_productId = -1;
    int m_defaultPrice = 0;
    QList<PriceLevel> m_definedLevels;
    QMap<int, QVariant> m_loadedPrices; // Harga asli dari DB (row index -> price)
    QMap<int, QVariant> m_editPrices;   // Perubahan harga (row index -> price)
};