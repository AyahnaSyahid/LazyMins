#pragma once

#include <QObject>

class OrderItemFinishingManager;
class QAbstractItemModel;
class FinishingControler : public QObject
{
    Q_OBJECT
public:
    explicit FinishingControler(QObject *parent = nullptr);
    ~FinishingControler();

    QAbstractItemModel *model() const;
private:
    OrderItemFinishingManager *m_orderItemFinishingManager;
    QAbstractItemModel *m_model;
};