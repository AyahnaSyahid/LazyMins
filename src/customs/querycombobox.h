#pragma once

#include <QComboBox>
#include <QTableView>
#include <QSqlQueryModel>
#include "src/managers/managers.h"

class QSqlQueryModel;
class QueryComboBox : public QComboBox
{
  Q_OBJECT
  public:
    QueryComboBox(QWidget * = nullptr);

    void setQuery(const QString& s, QSqlDatabase &db = BaseManager::connection);
    void setQuery(const QString& s, const QVariantMap &binding, QSqlDatabase &db = BaseManager::connection);
    int  findValue(const QVariant& value, int column = 0) const;
    int  findIndex(const QVariant& val, int column = 0) const;
    void showColumn(int column, bool show = true);
    
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