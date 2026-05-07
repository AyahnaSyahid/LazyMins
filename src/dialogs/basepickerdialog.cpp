#include "basepickerdialog.h"
#include "ui_basepickerdialog.h"
#include "src/managers/basemanager.h"

#include <QHeaderView>
#include <QSqlError>
#include <QDebug>
#include <QScrollBar>

namespace {
  
  class ProxyModel : public QSortFilterProxyModel {
  public:
      explicit ProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent) {}

      QVariant data(const QModelIndex &index, int role) const override {
          if (role == Qt::TextAlignmentRole) {
              QVariant value = sourceModel()->data(mapToSource(index), Qt::DisplayRole);
              // Jika data adalah angka (int, double, float), otomatis rata kanan
              if (value.typeId() == QMetaType::Int || value.typeId() == QMetaType::Double || 
                  value.typeId() == QMetaType::LongLong) {
                  return int (Qt::AlignRight | Qt::AlignVCenter);
              }
          }
          return QSortFilterProxyModel::data(index, role);
      }
  };
  
}

BasePickerDialog::BasePickerDialog(const QString &title, QWidget *parent)
    : QDialog(parent), ui(new Ui::BasePickerDialog)
{
    ui->setupUi(this);
    setWindowTitle(title);

    m_queryModel = new QSqlQueryModel(this);
    m_proxyModel = new ProxyModel(this);
    m_filterTimer = new QTimer(this);

    m_proxyModel->setSourceModel(m_queryModel);
    m_proxyModel->setFilterKeyColumn(-1);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    
    ui->baseView->setModel(m_proxyModel);
    ui->baseView->verticalHeader()->hide();

    m_filterTimer->setInterval(300);
    connect(ui->lineEdit, &QLineEdit::textChanged, m_filterTimer, qOverload<>(&QTimer::start));
    connect(m_filterTimer, &QTimer::timeout, this, &BasePickerDialog::onFilterTimerTimeout);

    connect(ui->baseView, &QTableView::clicked, this, &BasePickerDialog::onItemSelected);
    connect(ui->baseView, &QTableView::activated, this, &BasePickerDialog::onItemSelected);
}

BasePickerDialog::~BasePickerDialog() { delete ui; }

bool BasePickerDialog::setupData(const QString &query, const QMap<int, QString> &headers) {
    m_queryModel->setQuery(query, BaseManager::connection); // 
    
    // Pengecekan apakah query valid
    if (m_queryModel->lastError().isValid()) {
        qCritical() << "SQL Error dalam Picker: " << m_queryModel->lastError().text();
        qCritical() << "Query:" << query;
        return false;
    }

    for (auto it = headers.begin(); it != headers.end(); ++it) {
        m_queryModel->setHeaderData(it.key(), Qt::Horizontal, it.value());
    }
    
    ui->baseView->resizeColumnsToContents(); // 
    return true;
}

void BasePickerDialog::adjustDialogSize() {
    // 1. Hitung total lebar kolom yang terlihat
    int totalWidth = 0;
    int columnCount = m_proxyModel->columnCount();
    
    for (int i = 0; i < columnCount; ++i) {
        if (!ui->baseView->isColumnHidden(i)) {
            totalWidth += ui->baseView->columnWidth(i);
        }
    }

    // 2. Tambahkan margin layout dan lebar scrollbar (jika ada)
    int frameWidth = ui->baseView->frameWidth() * 2;
    int scrollbarWidth = ui->baseView->verticalScrollBar()->isVisible() ? 
                         ui->baseView->verticalScrollBar()->width() : 20;
    
    // Ambil margin dari layout utama
    int margins = layout()->contentsMargins().left() + layout()->contentsMargins().right();
    
    // Berikan sedikit padding tambahan agar tidak terlalu mepet
    int finalWidth = totalWidth + frameWidth + scrollbarWidth + margins + 10;

    // 3. Batasi ukuran minimum dan maksimum agar dialog tetap proporsional
    finalWidth = qBound(400, finalWidth, 1000); 
    
    this->resize(finalWidth, this->height());
}

void BasePickerDialog::setVerticalHeaderShown(bool shown)
{
    ui->baseView->verticalHeader()->setVisible(shown);
}

void BasePickerDialog::setHiddenColumns(const QList<int> &columns) {
    for (int col : columns) {
        ui->baseView->hideColumn(col); // 
    }
}

int BasePickerDialog::getIdFromIndex(const QModelIndex &index) {
    if (!index.isValid()) return -1;
    return index.siblingAtColumn(0).data().toInt(); // 
}

void BasePickerDialog::onItemSelected(const QModelIndex &index) {
    int id = getIdFromIndex(index);
    if (id != -1) {
        emit idPicked(id);
        accept();
    }
}

void BasePickerDialog::onFilterTimerTimeout() {
    m_proxyModel->setFilterFixedString(ui->lineEdit->text()); // 
}