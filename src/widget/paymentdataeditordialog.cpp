#include "paymentdataeditordialog.h"
#include "ui_paymentdataeditordialog.h"

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

PaymentDataEditorDialog::PaymentDataEditorDialog(int required, const QList<QSqlRecord> &_r, QWidget *p):
  recs(_r),
  recs_update {}, 
  itemModel(new QStandardItemModel), 
  ui(new Ui::PaymentDataEditorDialog),
  m_req(required), 
  QDialog(p)
{
  ui->setupUi(this);
  for(const auto &r : recs) {
    QList<QStandardItem*> row;
    auto admin = new QStandardItem();
    admin->setData(r.value("admin"), Qt::EditRole);
    admin->setEditable(true);
    auto amount = new QStandardItem();
    amount->setData(r.value("amount").toLongLong(), Qt::EditRole);
    amount->setEditable(true);
    auto method = new QStandardItem();
    method->setData(r.value("method"), Qt::EditRole);
    method->setEditable(true);
    auto paytime = new QStandardItem();
    paytime->setData(r.value("payment_time").toDateTime(), Qt::EditRole);
    paytime->setEditable(true);
    row << admin << amount << method << paytime;
    itemModel->appendRow(row);
  }
  ui->tableView->setModel(itemModel);
  itemModel->setHorizontalHeaderLabels( {"Admin", "Jumlah", "Metode", "Stamp"} );
  ui->tableView->setItemDelegateForColumn(1, new AmountDelegate(this));
  ui->tableView->setItemDelegateForColumn(3, new DatetimeDelegate(this));
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