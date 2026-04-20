#include "actiongroup.h"
#include "src/dialogs/akuntransaksidialog.h"
#include "src/dialogs/expensedialog.h"

ActionGroup::ActionGroup(QObject *parent):
QObject(parent)
{
  buatAkunTransaksiAction = new QAction("Akun Transaksi", this);
  buatAkunTransaksiAction->setObjectName("buatAkunTransaksiAction");
  
  catatPengeluaranAction = new QAction("Pengeluaran", this);
  catatPengeluaranAction->setObjectName("catatPengeluaranAction");
  
  QMetaObject::connectSlotsByName(this);
};

void ActionGroup::on_buatAkunTransaksiAction_triggered() {
  auto dl = new AkunTransaksiDialog(m_root);
  dl->prepareCreate();
  dl->setAttribute(Qt::WA_DeleteOnClose);
  dl->setWindowModality(Qt::ApplicationModal);
  connect(dl, &QDialog::accepted, this, &ActionGroup::newAkunTransaksiCreated);
  dl->exec();
}

void ActionGroup::on_catatPengeluaranAction_triggered() {
  ExpenseDialog ed(m_root);
  ed.exec();
}

void ActionGroup::setRootWidget(QWidget *r) { m_root = r; }