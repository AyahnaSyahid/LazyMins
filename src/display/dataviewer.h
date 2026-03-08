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

public slots:
    void refresh();
    void setFilter(const QString &filter);
private slots:
    void updateNavigation();

private:
    Ui::DataViewer *ui;
    AdvancedQueryModel m_model;
    QTimer m_filterTimer;
};