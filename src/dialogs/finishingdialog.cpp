#include "finishingdialog.h"
#include "ui_finishingdialog.h"

#include <QSqlQueryModel>
#include <QTableView>
#include <QHeaderView>
#include <QMessageBox>
#include <QTimer>

#include "src/managers/basemanager.h"

FinishingDialog::FinishingDialog(QWidget *parent) :
  QDialog(parent), ui(new Ui::FinishingDialog), m_finishingModel(new QSqlQueryModel(this)), m_mode(Mode::Create), m_item(nullptr)
{
  ui->setupUi(this);

  // setting up finishing_services model
  m_finishingModel->setQuery(R"--(
    SELECT id, code, name, description, price_per_unit, unit
    FROM finishing_services WHERE is_active = 1;
  )--", BaseManager::connection);
  
  while(m_finishingModel->canFetchMore()) m_finishingModel->fetchMore();
  ui->finishingComboBox->blockSignals(true);
  ui->finishingComboBox->setModel(m_finishingModel);
  ui->finishingComboBox->setModelColumn(2);
  ui->finishingComboBox->setCurrentIndex(-1);
  ui->finishingComboBox->blockSignals(false);
  
  // settup m_finishingView
  m_finishingView = new QTableView;
  m_finishingView->verticalHeader()->setMinimumSectionSize(18);
  m_finishingView->verticalHeader()->setDefaultSectionSize(20);
  m_finishingView->verticalHeader()->hide();
  m_finishingView->horizontalHeader()->hide();
  ui->finishingComboBox->setView(m_finishingView);

  m_finishingView->hideColumn(0); // hide column id
  m_finishingView->resizeColumnsToContents();
  m_finishingView->setMinimumWidth(m_finishingView->horizontalHeader()->length());

  connect(ui->finishingComboBox, &QComboBox::currentIndexChanged, this, [this]() {
    if (ui->finishingComboBox->currentIndex() != -1) {
      auto index = m_finishingModel->index(ui->finishingComboBox->currentIndex(), 0);
      int harga = index.siblingAtColumn(4).data().toInt();
      ui->hargaSpinBox->setValue(harga);
    } else {
      ui->hargaSpinBox->setValue(0);
    }
  });

  connect(ui->hargaSpinBox, &QSpinBox::valueChanged, this, &FinishingDialog::recalculate);
  connect(ui->qtySpinBox, &QSpinBox::valueChanged, this, &FinishingDialog::recalculate);
}

FinishingDialog::~FinishingDialog() {
  delete ui;
}

void FinishingDialog::recalculate() {
  ui->totalSpinBox->setValue(ui->hargaSpinBox->value() * ui->qtySpinBox->value());
}

void FinishingDialog::setItem(FinishingItem *item) {
  m_mode = Mode::Modify;
  m_item = item;
  
  // FIX: use .row() to get the combobox row index, not .data() which returns
  // the finishing_id (column 0 value) and would set the wrong combo index.
  auto indexes = m_finishingModel->match(m_finishingModel->index(0, 0), Qt::DisplayRole, item->finishing_id, 1, Qt::MatchExactly);
  if (!indexes.isEmpty()) {
    ui->finishingComboBox->setCurrentIndex(indexes.at(0).row());
  }
  ui->hargaSpinBox->setValue(item->finishing_price);
  ui->qtySpinBox->setValue(item->quantity);
};

void FinishingDialog::on_simpanButton_clicked() {
  if (ui->finishingComboBox->currentText().isEmpty() || ui->finishingComboBox->currentIndex() == -1) {
    QMessageBox::information(this, "Periksa masukkan", "Anda belum menentukan jenis finishing");
    return ;
  }
  auto modelIndex = m_finishingModel->index(ui->finishingComboBox->currentIndex(), 0);
  auto f_id = modelIndex.data().toInt();
  if (m_mode == Create) {
    FinishingItem item;
    item.finishing_id = f_id;
    item.finishing_name = ui->finishingComboBox->currentText();
    item.quantity = ui->qtySpinBox->value();
    item.finishing_price = ui->hargaSpinBox->value();
    emit createItem(item);
  } else {
    m_item->finishing_id = f_id;
    m_item->finishing_name = ui->finishingComboBox->currentText();
    m_item->quantity = ui->qtySpinBox->value();
    m_item->finishing_price = ui->hargaSpinBox->value();
  }
  accept();
}