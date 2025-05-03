#include "tablemanager.h"
#include <QSqlQuery>
#include <QSqlError>

TableManager::TableManager(const QString& tn, QObject* parent) : _tableName(tn), QObject(parent) {}

TableManager::~TableManager() {}

bool TableManager::insert(const QVariantMap& param) {
    if(_tableName.isEmpty())
        return false;
    if(_lastError.isValid())
        _lastError = QSqlError();
    QSqlQuery q;
    QString queryString("INSERT INTO %1 (%2) VALUES (%3)");
    QString columnList = param.keys().join(", ");
    QStringList qMark;
    for(int n=0; n<param.count(); ++n)
        qMark << "?";

    QVariantList values = param.values();
    
    queryString = queryString.arg(_tableName, columnList, qMark.join(", "));
    q.prepare(queryString);
    for(auto v = values.cbegin(); v != values.cend(); ++v)
        q.addBindValue(*v);

    bool ex_ok = q.exec();

    if( ! ex_ok ) {
        _lastError = q.lastError();
        emit error(_lastError);
        return false;
    }
    
    emit inserted(q.lastInsertId());
    return true;
}

bool TableManager::update(const QVariantMap& which, const QVariantMap& value) {
    if(_tableName.isEmpty())
        return false;
    if(_lastError.isValid())
        _lastError = QSqlError();
    QString updateString("UPDATE %1 SET (%2) = (%3) WHERE (%4) = (%5)");
    QStringList setKey = value.keys(),
                whichKey = which.keys();
    QString setColumns = setKey.join(", "),
            whichColumns = whichKey.join(", ");
   
    auto setValueList = value.values();
    auto whichValueList = which.values();
    
    QStringList setPlh, whichPlh;
    for(auto sv = setValueList.cbegin(); sv != setValueList.cend(); sv++)
        setPlh << "?";
    for(auto sv = whichValueList.cbegin(); sv != whichValueList.cend(); sv++)
        whichPlh << "?";
    QString prepareQuery = updateString.arg(tableName(), setColumns, setPlh.join(", "), whichColumns, whichPlh.join(", "));
    QSqlQuery q;
    q.prepare(prepareQuery);
    for(auto vv = setValueList.cbegin(); vv != setValueList.cend(); ++vv) {
        q.addBindValue(*vv);
    }
    for(auto vv = whichValueList.cbegin(); vv != whichValueList.cend(); ++vv) {
        q.addBindValue(*vv);
    }

    bool ex_ok = q.exec();
    if(ex_ok) {
        emit updated(1);
        return true;
    }
    
    if(q.lastError().isValid()) {
        _lastError = q.lastError();
        emit error(_lastError);
    }
    return false;
}

bool TableManager::erase(const QVariantMap& param) {
    if(_tableName.isEmpty())
        return false;
    if(_lastError.isValid())
        _lastError = QSqlError();
    QString eraseString("DELETE FROM %1 WHERE (%2) = (%3)");
    QStringList ph;
    for(int i=0; i<param.count(); ++i)
    ph << "?";
    QSqlQuery q;
    q.prepare(eraseString.arg(tableName(), param.keys().join(", "), ph.join(", ")));
    auto pValues = param.values();
    for(auto vv=pValues.cbegin(); vv!=pValues.cend(); ++vv)
        q.addBindValue(*vv);
    bool ex_ok = q.exec();
    if(ex_ok) {
        emit updated(q.numRowsAffected());
        return true;
    }
    if(q.lastError().isValid()) {
        _lastError = q.lastError();
        emit error(_lastError);
    }
    return false;
}

QSqlRecord TableManager::record() const {
    if(!_tableName.isEmpty())
        return QSqlDatabase::database().record(_tableName);
    return QSqlRecord();
}