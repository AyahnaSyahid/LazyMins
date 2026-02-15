#ifndef CUSTOMERTABLE_H
#define CUSTOMERTABLE_H


#include "realtimedatawidget.h"
#include <QTableView>
#include <QLineEdit>
#include <QSqlQueryModel>
#include <QStyledItemDelegate>

class CustomerContactModel;
class CustomerView : public RealTimeDataWidget
{
  Q_OBJECT
  
  public:
    explicit CustomerView(QWidget *parent=nullptr);
    ~CustomerView();
  
  public slots:
    void reloadModelData(const QList<QString> &tables) override;
  
  private:
    QTableView *c_view;
    CustomerContactModel *c_model;
    QLineEdit *filterEdit;
};

class CustomerContactModel : public QSqlQueryModel
{
  public:
    explicit CustomerContactModel(QObject *parent=nullptr) : QSqlQueryModel(parent) {}
    ~CustomerContactModel() {}
    bool setData(const QModelIndex& mi, const QVariant &val, int role=Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& mi) const override;
};

#endif