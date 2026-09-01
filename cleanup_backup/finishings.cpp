#include "finishings.h"
#include "src/managers/orderitemfinishingmanager.h"
#include <QSqlQueryModel>

FinishingControler::FinishingControler(QObject *parent) : QObject(parent), m_orderItemFinishingManager(new OrderItemFinishingManager), m_model(new QSqlQueryModel(this))
{}

FinishingControler::~FinishingControler()
{
    delete m_orderItemFinishingManager;
}

QAbstractItemModel *FinishingControler::model() const
{
    return m_model;
}
