#include "finishingdialog.h"
#include "ui_finishingdialog.h"

#include <QSqlQueryModel>
#include <QTableView>
#include <QHeaderView>

#include "src/managers/basemanager.h"

FinishingDialog::FinishingDialog(QWidget *parent) :
  ui(new Ui::FinishingDialog), m_finishingModel(new QSqlQueryModel(this)), QDialog(parent)
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

  connect(ui->hargaSpinBox, &QSpinBox::valueChanged, this, FinishingDialog::recalculate);
  connect(ui->qtySpinBox, &QSpinBox::valueChanged, this, FinishingDialog::recalculate);
}

FinishingDialog::~FinishingDialog() {
  delete ui;
}

void FinishingDialog::recalculate() {
  ui->totalSpinBox->setValue(ui->hargaSpinBox->value() * ui->qtySpinBox->value());
}