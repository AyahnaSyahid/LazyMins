#pragma once

#include <QWidget>
#include "src/models/advancedquerymodel.h"
#include <QTimer>

namespace Ui {
    class DataViewer;
}

class DataViewer : public QWidget
{
    Q_OBJECT
public:
    DataViewer(QWidget *parent = nullptr);
    virtual ~DataViewer();
    void setQueryArgs(const QString &query, const QVariantMap &bindings = {});
    void setPageSize(int size) { m_model.setPageSize(size); }
    void setColumnVisible(const QString& name, bool vis);
    void setColumnVisible(int col, bool vis);
    void setFilterColumnNames(const QStringList& sl);
    void setEditable(bool editable);
public slots:
    void refresh();
    void setFilter(const QString &filter);
    
private slots:
    void updateNavigation();

signals:
    void refreshed();

protected:
  Ui::DataViewer *Ui() { return ui; }
  AdvancedQueryModel &model() {return m_model; }

private:
    QHash<int, QString> m_columns;
    Ui::DataViewer *ui;
    AdvancedQueryModel m_model;
    QTimer m_filterTimer;
    QStringList m_filterColumnNames;
};