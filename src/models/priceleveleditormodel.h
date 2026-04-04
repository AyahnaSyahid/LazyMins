#pragma once

#include <QAbstractTableModel>

class PriceLevelEditorModel : public QAbstractTableModel
{
  Q_OBJECT
  
  public:
    explicit PriceLevelEditorModel(QObject * = nullptr);
    ~PriceLevelEditorModel();
    
    bool setProductId(int pid);
    
    // Overrides
    int rowCount(const QModelIndex& = QModelIndex()) const override;
    int columnCount(const QModelIndex& = QModelIndex()) const override { return 2; }
    
    QVariant data(const QModelIndex& ix, int role = Qt::DislayRole) const override;
    bool setData(const QModelIndex& ix, int role = Qt::EditRole) override;
    
    Qt::ItemFlags flags(const QModelIndex& ix) const override;

  private:
    int m_productId;
    
    struct PriceLevelItem {
      int id;
      QString level_name, description;
    };
    
    QList<PriceLevelItem> m_definedLevels;
};