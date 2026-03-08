#include "dataviewer.h"
#include "ui_dataviewer.h"

#include <QPushButton>
#include <QTimer>
#include "src/managers/managers.h"

DataViewer::DataViewer(QWidget *parent) : ui(new Ui::DataViewer), m_model(this), m_filterTimer(this), QWidget(parent)
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
    QTimer::singleShot(0, this, &DataViewer::updateNavigation);
}

DataViewer::~DataViewer()
{
    delete ui;
}

void DataViewer::setQueryArgs(const QString &query, const QVariantMap &bindings)
{
    m_model.setQueryArgs(query, bindings);
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
}

void DataViewer::setFilter(const QString &filter)
{
    m_model.setFilter(filter);
    updateNavigation();
}