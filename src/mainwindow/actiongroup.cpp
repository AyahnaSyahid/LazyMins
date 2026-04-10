#include "actiongroup.h"
#include "src/dialogs/akuntransaksidialog.h"

ActionGroup::ActionGroup(QObject *parent):
QObject(parent)
{
  buatAkunTransaksiAction = new QAction(this);
  buatAkunTransaksiAction->setObjectName("buatAkunTransaksiAction");
  buatAkunTransaksiAction->setText("Akun Transaksi");
  
  QMetaObject::connectSlotsByName(this);
};

void ActionGroup::on_buatAkunTransaksiAction_triggered() {
  auto dl = new AkunTransaksiDialog();
  dl->prepareCreate();
  dl->setAttribute(Qt::WA_DeleteOnClose);
  connect(dl, &QDialog::accepted, this, &ActionGroup::newAkunTransaksiCreated);
  dl->open();
}