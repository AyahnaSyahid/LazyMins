#include "expenseitem.h"

ExpenseItem::ExpenseItem(qint64 eid)
: expense_id(eid), DatabaseItem()
{
    q.prepare("SELECT * FROM expenses WHERE expense_id = ?");
    q.addBindValue(eid);
    if(q.exec() && q.next()) {
        qValueAssign(division_id, qint64);
        qValueAssign(expense_date, QDateTime);
        expense_date = expense_date.toLocalTime();
        qValueAssign(amount, qint64);
        qValueAssign(description, QString);
        qValueAssign(category, QString);
        qValueAssign(created_by, qint64);
        qValueAssign(created_date, QDateTime);
        created_date = created_date.toLocalTime();
        qValueAssign(modified_by, qint64);
        qValueAssign(modified_date, QDateTime);
        modified_date = modified_date.toLocalTime();
    }
}

ExpenseItem::~ExpenseItem() {}

bool ExpenseItem::save() {
    if(expense_id < 1) {
        q.prepare(R"-(
            INSERT INTO expenses (division_id, expense_date, amount, description,
                category, created_by, created_date)
            VALUES (?, ?, ?, ?, ?, ?, ?);
        )-");
        q.addBindValue(division_id < 1 ? QVariant() : division_id);
        q.addBindValue(expense_date.isValid() ? expense_date.toUTC() : QVariant());
        q.addBindValue(amount);
        q.addBindValue(description);
        q.addBindValue(category);
        q.addBindValue(created_by < 1 ? QVariant() : created_by);
        q.addBindValue(QDateTime::currentDateTimeUtc());
        if(!q.exec()) {
            debugError(q);
            return false;
        } else {
            expense_id = q.lastInsertId().toLongLong();
            return true;
        }
    } else {
        q.prepare(R"-(
            UPDATE expenses SET ( division_id, expense_date, amount, description,
                category, modified_by, modified_date)
                = (?, ?, ?, ?, ?, ?, ?) WHERE expense_id = ?;
        )-");
        q.addBindValue(division_id < 1 ? QVariant() : division_id);
        q.addBindValue(expense_date.isValid() ? expense_date.toUTC() : QVariant());
        q.addBindValue(amount);
        q.addBindValue(description);
        q.addBindValue(category);
        q.addBindValue(modified_by < 1 ? QVariant() : modified_by);
        q.addBindValue(modified_date.isValid() ? modified_date.toUTC() : QVariant());
        q.addBindValue(expense_id);
        if(!q.exec()) {
            debugError(q);
            return false;
        }
        return true;
    }
    return false;
}

bool ExpenseItem::erase()
{
    q.prepare("DELETE FROM expenses WHERE expense_id = ?");
    q.addBindValue(expense_id);
    if(!q.exec()) {
        debugError(q);
        return false;
    }
    return true;
}