#include "paymentdataeditordialog.h"
#include "ui_paymentdataeditordialog.h"
#include "../databaseinterface.h"
#include <QDateTime>
#include <QStyledItemDelegate>
#include <QSpinBox>
#include <QDateTimeEdit>
#include <QMenu>
#include <QAction>

namespace {
  class AmountDelegate : public QStyledItemDelegate {
    public:
      AmountDelegate(QObject * p= nullptr) : QStyledItemDelegate(p) {}
      ~AmountDelegate() {}
      QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &opt, const QModelIndex& mi) const override {
        auto e = new QSpinBox(parent);
        return e;
      }
      void setEditorData(QWidget *ed, const QModelIndex& mi) const override {
        QSpinBox *led = qobject_cast<QSpinBox*>(ed);
        if (led) {
          led->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
          led->setButtonSymbols(QSpinBox::NoButtons);
          led->setGroupSeparatorShown(true);
          led->setMinimum(-100'000'000);
          led->setMaximum(100'000'000);
          led->setSingleStep(1'000);
          led->setAccelerated(true);
          led->setValue(mi.data(Qt::EditRole).toLongLong());
          return ;
        }
        QStyledItemDelegate::setEditorData(ed, mi);
      }
      QString displayText(const QVariant& value, const QLocale &loc) const {
        return QLocale().toString(value.toLongLong());
      }
      
    protected:
      void initStyleOption(QStyleOptionViewItem *opt, const QModelIndex &mi) const override {
        QStyledItemDelegate::initStyleOption(opt, mi);
        opt->displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
      }
  };
  
  class DatetimeDelegate : public QStyledItemDelegate {
    public:
      DatetimeDelegate(QObject *p=nullptr) : QStyledItemDelegate(p) {}
      ~DatetimeDelegate() {}
      
      void setEditorData(QWidget *ed, const QModelIndex& mi) const override {
        QStyledItemDelegate::setEditorData(ed, mi);
        auto p = qobject_cast<QDateTimeEdit*> (ed);
        if (p) {
          p->setAlignment(Qt::AlignCenter);
          p->setCalendarPopup(true);
          p->setDisplayFormat("dd / MM / yyyy");
          return ;
        }
      }

    protected:
      void initStyleOption(QStyleOptionViewItem *opt, const QModelIndex &mi) const override {
        QStyledItemDelegate::initStyleOption(opt, mi);
        opt->displayAlignment = Qt::AlignCenter;
      }
      
  };
}

PaymentDataEditorDialog::PaymentDataEditorDialog(int invoice_id, QWidget *p):
  ui(new Ui::PaymentDataEditorDialog),
  m_id(invoice_id),
  itemModel(new PaymentDataEditorModel(this)),
  QDialog(p)
{
  ui->setupUi(this);
  itemModel->setQueryArgs(
  "SELECT id, admin, method, amount, payment_time "
    "FROM payments WHERE invoice_id = :i_id AND status = 'active'", 
    {{":i_id", invoice_id}},
    DatabaseInterface::instance().database()
    );
  itemModel->setReadOnlyColumn(0);
  ui->tableView->setModel(itemModel);
  ui->tableView->setItemDelegateForColumn(4, new DatetimeDelegate(this));
  ui->tableView->setItemDelegateForColumn(3, new AmountDelegate(this));
  ui->tableView->hideColumn(0);
}

PaymentDataEditorDialog::~PaymentDataEditorDialog() { delete ui; }


QList<PaymentData> PaymentDataEditorDialog::getPaymentsData() const
{
  QList<PaymentData> pdl;
  for(int row = 0; row < itemModel->rowCount(); ++row) {
    PaymentData pd;
    pd.adminName = itemModel->index(row, 0).data(Qt::EditRole).toString();
    pd.amount = itemModel->index(row, 1).data(Qt::EditRole).toInt();
    pd.method = itemModel->index(row, 2).data(Qt::EditRole).toString();
    pd.paymentTime = itemModel->index(row, 3).data(Qt::EditRole).toDateTime();
    pdl << pd;
  }
  return pdl;
}

void PaymentDataEditorDialog::on_tableView_customContextMenuRequested(const QPoint& p)
{
  auto mi = ui->tableView->indexAt(p);
  if (! mi.isValid() ) return;
  QMenu cmenu;
  auto actInsert = cmenu.addAction("Baru");
  auto actDelete = cmenu.addAction("Hapus");
  cmenu.exec(ui->tableView->viewport()->mapToGlobal(p));
}

void PaymentDataEditorDialog::on_simpanButton_clicked() {
  if (itemModel->submitAll()) {
    accept();
  }
  qDebug() << itemModel->lastError();
}