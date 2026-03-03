#pragma once

#include <QComboBox>
#include <QTableView>
#include <QSqlQueryModel>
#include "src/database/databasemanager.h"

class QSqlQueryModel;
class QueryComboBox : public QComboBox
{
  Q_OBJECT
  public:
    QueryComboBox(QWidget *p=nullptr);

    void setQuery(const QString& s, QSqlDatabase &db = DatabaseManager::instance().database());
    void setQuery(const QString& s, const QVariantMap &binding, QSqlDatabase &db = DatabaseManager::instance().database());
    int findValue(const QVariant& value, int column = 0) const;
  public slots:
    virtual void refetchData();
    virtual void boxViewAutoResize();
  
  private:
    void popError(const QString& err) const;

  protected:
    QSqlQueryModel *qmodel;
    QTableView *boxView;
    QString m_query;
    QVariantMap m_bindings;
    QSqlDatabase m_db;
};