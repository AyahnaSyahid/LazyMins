#include "dataviewer.h"
#include "ui_dataviewer.h"

#include <QPushButton>
#include <QTimer>
#include "src/managers/managers.h"
#include "src/customs/booleandelegate.h"

DataViewer::DataViewer(QWidget *parent) : ui(new Ui::DataViewer), 
  m_model(this), m_filterTimer(this), m_columns {}, 
  m_filterColumnNames {}, QWidget(parent)
{
    ui->setupUi(this);
    ui->dataView->setModel(&m_model);
    connect(&m_model, &QAbstractItemModel::rowsInserted, this, &DataViewer::updateNavigation);
    connect(&m_model, &QAbstractItemModel::rowsRemoved, this, &DataViewer::updateNavigation);
    connect(ui->firstButton, &QPushButton::clicked, [=]
            {m_model.setPage(1); refresh(); });
    connect(ui->nextButton, &QPushButton::clicked, [=]
            {m_model.setPage(m_model.currentPage() + 1); refresh(); });
    connect(ui->prevButton, &QPushButton::clicked, [=]
            {m_model.setPage(m_model.currentPage() - 1); refresh(); });
    connect(ui->lastButton, &QPushButton::clicked, [=]
            {m_model.setPage(m_model.totalPages()); refresh(); });
    m_filterTimer.setSingleShot(true);
    m_filterTimer.setInterval(500);
    connect(ui->filterEdit, &QLineEdit::textChanged, [this]{m_filterTimer.start();});
    connect(&m_filterTimer, &QTimer::timeout, [this]{
        setFilter(ui->filterEdit->text());
    });
    connect(&m_model, &QAbstractItemModel::modelReset, this, &DataViewer::updateNavigation);
    // ui->dataView->setItemDelegateForColumn(10, new BooleanDelegate(this));
    QTimer::singleShot(0, this, &DataViewer::updateNavigation);
}

void DataViewer::setFilterColumnNames(const QStringList& sl) {
  m_filterColumnNames.clear();
  m_filterColumnNames = sl;
}


DataViewer::~DataViewer()
{
    delete ui;
}

void DataViewer::setQueryArgs(const QString &query, const QVariantMap &bindings)
{
    m_model.setQueryArgs(query, bindings);
    m_columns.clear();
    auto m_record = m_model.record();
    for(int i=0; i < m_record.count(); ++i) {
      m_columns.insert(i, m_record.fieldName(i));
    }
}

void DataViewer::updateNavigation()
{
    if (m_model.totalPages() < 2)
    {
        ui->navigationFrame->hide();
        return;
    }
    ui->navigationFrame->show();
    ui->firstButton->setEnabled(m_model.currentPage() != 1);
    ui->prevButton->setEnabled(m_model.currentPage() > 1);
    ui->nextButton->setEnabled(m_model.currentPage() < m_model.totalPages());
    ui->lastButton->setEnabled(m_model.currentPage() != m_model.totalPages());
    ui->currentPage->setValue(m_model.currentPage());
    ui->labelTotal->setText(QString("/%2").arg(m_model.totalPages()));
}

void DataViewer::refresh()
{
    m_model.refresh();
    updateNavigation();
    emit refreshed();
}

void DataViewer::setFilter(const QString &filter)
{
  if (m_filterColumnNames.isEmpty()) return ;
  QStringList filters;
  for(auto const& fname : m_filterColumnNames) {
    filters << QString(" %1 LIKE '%%2%' ").arg(fname, filter);
  }
  m_model.setFilter(filters.join("OR"));
  updateNavigation();
}

void DataViewer::setColumnVisible(const QString& name, bool vis) {
  for(auto const &[kk, kv] : m_columns.asKeyValueRange()) {
    if ( kv == name ) {
      setColumnVisible(kk, vis);
      return ;
    }
  }
}

void DataViewer::setColumnVisible(int col, bool vis) {
  if (col >= 0 && col < model().columnCount()) {
    if(vis)
      ui->dataView->showColumn(col);
    else
      ui->dataView->hideColumn(col);
  }
}

void DataViewer::setEditable(bool editable) {
  ui->dataView->setEditTriggers(editable ? QTableView::AllEditTriggers : QTableView::NoEditTriggers);
}