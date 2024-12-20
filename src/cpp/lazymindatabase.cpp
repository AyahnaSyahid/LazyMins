#include "lazymindatabase.h"
#include "productitem.h"
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QtDebug>

namespace DB_INIT {
  bool initializeDatabase();
}

LazyMinDatabase::LazyMinDatabase(QObject* parent)
    : QObject(parent)
{}

LazyMinDatabase::~LazyMinDatabase()
{}

int LazyMinDatabase::addProduct(const StructProduct& pr) {
    ProductItem pi(pr.code);
    pi.name = pr.name;
    pi.description = pr.desc;
    pi.cost = pr.pcost;
    pi.price = pr.psell;
    pi.use_area = pr.area;
    pi.qr_bar = pr.qr;
    pi.save();
    return pi.product_id;
}

bool LazyMinDatabase::setProduct(int id, const StructProduct&) {
    return false;
    // emit productModified(StructProduct&);
}

bool LazyMinDatabase::removeProduct(int id) {
    return ProductItem(id).erase();
}