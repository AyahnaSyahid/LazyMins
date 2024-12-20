#include "modelpembayarannota.h"
#include <QLocale>
#include <QtMath>
#include <QtDebug>


QString ModelPembayaranNota::defaultQuery("SELECT notaId AS Nota, "
              "customer AS Konsumen, "
              "sellPrice AS TotalHarga, "
              "money AS TotalBayar, "
              "disc as Diskon, "
              "CAST((sellPrice - disc) - money AS INT) as Sisa "
         "FROM (SELECT id as notaId, disc FROM payment) notaSelector, "
              "(SELECT customer_id, payment_id, SUM(sizeW * sizeH * quantity * sellprice) as sellPrice FROM sale GROUP BY payment_id HAVING payment_id IS NOT NULL) saleSelector ON saleSelector.payment_id = notaSelector.notaId, "
              "(SELECT id as cid, name as customer FROM customer) customerSelector ON saleSelector.customer_id = cid, "
              "(SELECT payment_id, SUM(cash + trf) as money FROM repayment GROUP BY payment_id HAVING payment_id IS NOT NULL) paymentSelector ON paymentSelector.payment_id = notaId");

ModelPembayaranNota::ModelPembayaranNota(QObject* parent)
    : QSqlQueryModel(parent), showLunas(false), orderDesc(false)
{
   setQuery(generatedQuery());
}

ModelPembayaranNota::~ModelPembayaranNota(){}

QVariant ModelPembayaranNota::data(const QModelIndex& mi, int role) const {
    if(role == Qt::TextAlignmentRole) {
        switch (mi.column()) {
            case 0:
            case 2:
            case 3:
            case 4:
            case 5:
                return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
            default:
                return QVariant();
        }
    } else if (role == Qt::DisplayRole) {
        QVariant _def = QSqlQueryModel::data(mi, role);
        switch (mi.column()) {
            case 0:
                return QString("%1").arg(_def.toString(), 8, QChar('0'));
            case 2:
            case 3:
            case 4:
            case 5:
                return QLocale().toString(_def.toInt());
            default:
                return _def;
        }
    }
    return QSqlQueryModel::data(mi, role);
}

void ModelPembayaranNota::lunasToggle(bool o) {
    showLunas = o;
    setQuery(generatedQuery());
    // qDebug() << generatedQuery();
}

void ModelPembayaranNota::orderToggle(bool o) {
    orderDesc = o;
    setQuery(generatedQuery());
    // qDebug() << generatedQuery();
}

void ModelPembayaranNota::setFilterName(const QString& s)
{
    filterName = s;
    setQuery(generatedQuery());
}
void ModelPembayaranNota::setFilterId(const QString& s)
{
    filterId = s;
    setQuery(generatedQuery());
}

QString ModelPembayaranNota::generatedQuery() const {
    QStringList filters;
    QString fr(defaultQuery);
    if(!filterName.isEmpty()) {
        filters << QString("Konsumen LIKE '%%1%'").arg(filterName);
    }
    if(!filterId.isEmpty()) {
        filters << QString("Nota LIKE '%%1%'").arg(filterId);
    }
    if(!showLunas) {
        filters << QString("Sisa > 0");
    }
    
    
    if(filters.count()) {
        fr += QString(" WHERE %1").arg(filters.join(" AND "));
    }
    
    if(orderDesc) {
        return fr + " ORDER BY notaId DESC";
    }
    return fr + " ORDER BY notaId ASC";
}

