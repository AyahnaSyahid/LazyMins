#include "divisionitem.h"

DivisionItem::DivisionItem(qint64 did)
: division_id(did) {
    q.prepare("SELECT * FROM divisions WHERE division_id = ?");
    q.addBindValue(did);
    if(q.exec() && q.next()) {
        qValueAssign(division_name, QString);
        qValueAssign(created_date, QDateTime);
    } else {
        debugError(q);
        division_id = -1;
    }
}

DivisionItem::DivisionItem(const QString& uname)
: division_name(uname) {
    q.prepare("SELECT * FROM divisions WHERE division_name = ? COLLATE NOCASE;");
    q.addBindValue(uname);
    if(q.exec() && q.next()) {
        qValueAssign(division_id, qint64);
        qValueAssign(division_name, QString);
        qValueAssign(created_date, QDateTime);
    }  else {
        debugError(q);
        division_id = -1;
    }
}

DivisionItem::~DivisionItem(){}

bool DivisionItem::save()
{
    if(division_id < 1) {
        q.prepare("INSERT INTO divisions (division_name) VALUES (?)");
        q.addBindValue(division_name);
        if(!q.exec()) {
            debugError(q);
            return false;
        }
        *this = DivisionItem(q.lastInsertId().toLongLong());
    } else {
        q.prepare("UPDATE divisions SET division_name = ? WHERE division_id = ?;");
        q.addBindValue(division_name);
        q.addBindValue(division_id);
        if(!q.exec()) {
            debugError(q);
            return false;
        }
    }
    return true;
}

bool DivisionItem::erase() {
    q.prepare("DELETE FROM divisions WHERE division_id = ?");
    q.addBindValue(division_id);
    if(!q.exec()) {
        debugError(q);
        return false;
    }
    return true;
}
