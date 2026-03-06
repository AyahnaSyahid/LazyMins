#include "orderdialog.h"
#include "ui_orderdialog.h"

#include "src/customs/flexibledelegate.h"
#include "src/dialogs/konsumenpickerdialog.h"
#include "src/dialogs/orderitemdialog.h"
#include <QHeaderView>
#include <QSqlTableModel>

namespace
{
  const QHash<int, QString> Column{
      {0, "id"},
      {1, "order_id"},
      {2, "product_id"},
      {3, "product_name"},
      {4, "sku"},
      {5, "quantity"},
      {6, "unit"},
      {7, "size_width"},
      {8, "size_height"},
      {9, "sale_price"},
      {10, "base_price"},
      {11, "discount_percentage"},
      {12, "discount_amount"},
      {13, "subtotal"},
      {14, "notes"},
      {15, "created_at"},
      {16, "updated_at"},
  };

  class ProductDelegate : public QStyledItemDelegate
  {
  public:
    explicit ProductDelegate(QObject *parent = nullptr) : m_productModel(new QSqlTableModel(this)), QStyledItemDelegate(parent)
    {
      m_productModel->setTable("products");
      m_productModel->select();
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
      auto editor = new QueryComboBox(parent);
      editor->setQuery("SELECT id, name, description, use_area, cost_price FROM products");
      editor->showColumn(3, false); // Sembunyikan kolom use_area
      editor->showColumn(4, false); // Sembunyikan kolom cost_price
      editor->setModelColumn(1);    // Tampilkan nama produk di combo box
      editor->setEditable(true);
      editor->boxViewAutoResize();
      return editor;
    }
    void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
      auto combo = qobject_cast<QueryComboBox *>(editor);
      if (combo)
      {
        int productId = index.data().toInt();
        int currentIndex = combo->findValue(productId);
        combo->setCurrentIndex(currentIndex);
      }
    }
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
    {
      auto combo = qobject_cast<QueryComboBox *>(editor);
      if (combo)
      {
        auto qmod = combo->model();
        int selectedRow = combo->currentIndex();
        if (selectedRow >= 0)
        {
          int productId = qmod->index(selectedRow, 0).data().toInt();
          QString productName = qmod->index(selectedRow, 1).data().toString();
          model->setData(index, productId); // Simpan product_id di model
          // model->setData(index.siblingAtColumn(3), productName); // Simpan nama produk di kolom tersembunyi
        }
      }
    }
    QString displayText(const QVariant &value, const QLocale &locale) const override
    {
      // Tampilkan nama produk berdasarkan product_id
      int productId = value.toInt();
      if (productId == 0)
        return ""; // Jika belum dipilih, tampilkan kosong
      int row = m_productModel->match(m_productModel->index(0, 0), Qt::DisplayRole, productId, 1, Qt::MatchExactly).value(0).row();
      if (row >= 0)
      {
        return m_productModel->index(row, 2).data().toString(); // Tampilkan nama produk
      }
      return QString("Unknown Product (ID: %1)").arg(productId);
    }

  private:
    QSqlTableModel *m_productModel;
  };

}

OrderDialog::OrderDialog(QWidget *p) : ui(new Ui::OrderDialog), emodel(new OrderItemEditorModel(this)), FormDialog(p)
{
  ui->setupUi(this);

  ui->orderItemView->setModel(emodel);
  ui->orderItemView->hideColumn(0);
  ui->orderItemView->hideColumn(1);
  ui->orderItemView->hideColumn(2);
  ui->orderItemView->hideColumn(4);
  ui->orderItemView->hideColumn(10);
  ui->orderItemView->hideColumn(11);
  ui->orderItemView->hideColumn(12);
  ui->orderItemView->hideColumn(14);
  ui->orderItemView->hideColumn(15);
  ui->orderItemView->hideColumn(16);
  ui->orderItemView->horizontalHeader()->setStretchLastSection(true);

  ui->orderItemView->setItemDelegateForColumn(2, new ProductDelegate(this));
  ui->orderItemView->addAction(ui->tambahItem);

  ui->orderNumberLineEdit->setText(OrderManager::generateOrderNumber());
  ui->tOrderDateTimeEdit->setDateTime(QDateTime::currentDateTime());
  ui->dLineDateTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(1));

  auto numberDelegate = FlexibleDelegate::create(
      {.displayer = [](const QVariant &value, const QLocale &locale)
       { return QString("%L1").arg(value.toInt()); },
       .styler = [](QStyleOptionViewItem &option, const QModelIndex &index)
       {
        if (index.column() == 5) { // Kolom quantity
            option.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
        } else if (index.column() == 7 || index.column() == 8) { // Kolom sale_price dan base_price
            option.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
        }
        option.locale = QLocale(QLocale::Indonesian, QLocale::Indonesia); }},
      ui->orderItemView);
  // set delegate untuk kolom quantity, sale_price, base_price, size_width, size_height
  ui->orderItemView->setItemDelegateForColumn(5, numberDelegate);
  ui->orderItemView->setItemDelegateForColumn(7, numberDelegate);
  ui->orderItemView->setItemDelegateForColumn(8, numberDelegate);
  ui->orderItemView->setItemDelegateForColumn(9, numberDelegate);
  ui->orderItemView->setItemDelegateForColumn(13, numberDelegate);
}

OrderDialog::~OrderDialog() { delete ui; }

void OrderDialog::setupFields()
{
  setFields({{ui->orderNumberLineEdit, "order_number"},
             {ui->konsumenLineEdit, "customer_name"},
             {ui->kontakLineEdit, "customer_phone"},
             {ui->statusLineEdit, "status"},
             {ui->prioritasLineEdit, "priority"},
             {ui->catatan1TextEdit, "notes"},
             {ui->catatan2TextEdit, "internal_notes"},
             {ui->subtotalSpinBox, "subtotal"},
             {ui->diskonDoubleSpinBox, "discount_percentage"},
             {ui->diskonRpSpinBox, "discount_amount"},
             {ui->pajakRpSpinBox, "tax_amount"},
             {ui->totalSpinBox, "total_amount"}});
}

void OrderDialog::setupBoundFields()
{
  addBoundField("order_date", [this]()
                { auto local = ui->tOrderDateTimeEdit->dateTime();
            return local.toTimeZone(QTimeZone::UTC); }, [this](QVariant val)
                { 
            auto local = val.toDateTime();
            local.setTimeZone(QTimeZone::LocalTime);
            ui->tOrderDateTimeEdit->setDateTime(local); });
  addBoundField("deadline_date", [this]()
                { auto local = ui->dLineDateTimeEdit->dateTime();
            return local.toTimeZone(QTimeZone::UTC); }, [this](QVariant val)
                { 
            auto local = val.toDateTime();
            local.setTimeZone(QTimeZone::LocalTime);
            ui->dLineDateTimeEdit->setDateTime(local); });
}

bool OrderDialog::onSave(const QVariantMap &mp) { return false; }

void OrderDialog::onPrepareCreate()
{
  ui->orderNumberLineEdit->setText(OrderManager::generateOrderNumber());
}

void OrderDialog::onPrepareModify(const QSqlRecord &orderRecord)
{
  emodel->loadFromOrder(orderRecord.value("id").toInt());
}

void OrderDialog::onOrderItemDialogAccepted()
{
  OrderItemDialog *editor = qobject_cast<OrderItemDialog *>(sender());
  QVariantMap itemData = editor->getFieldData();
  editor->prepareCreate(); // reset form untuk input berikutnya
  emodel->appendRow(itemData);
}

void OrderDialog::on_cariButton_clicked()
{
  // buat dialog pencarian konsumen (CustomerSearchDialog)
  // setelah konsumen dipilih, set nama dan kontak di form ini
  auto dialog = new KonsumenPickerDialog(this);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setWindowFlags(dialog->windowFlags() | Qt::FramelessWindowHint);
  auto buttonGeo = ui->cariButton->geometry();
  auto globalPos = mapToGlobal(buttonGeo.topLeft());
  dialog->move(globalPos);

  connect(dialog, &KonsumenPickerDialog::konsumenPicked, [this](const QSqlRecord &record)
          {
    ui->konsumenLineEdit->setText(record.value("nama_lengkap").toString());
    ui->kontakLineEdit->setText(record.value("nomor_telp").toString());
    // simpan price level untuk digunakan di OrderItemDialog
    int priceLevelId = record.value("pl_id").toInt();
    // qDebug() << "Selected price level ID:" << priceLevelId;
    ui->priceLevelComboBox->setLevelID(priceLevelId); });
  dialog->open();
}

void OrderDialog::on_tambahItem_triggered()
{
  auto editor = new OrderItemDialog(this);
  editor->setAttribute(Qt::WA_DeleteOnClose);
  editor->prepareCreate();
  editor->setAutoCommit(false);
  // get level harga pelanggan
  auto priceLevel = ui->priceLevelComboBox->currentId();
  editor->setCustomerPriceLevel(priceLevel);
  editor->open();
  connect(editor, &OrderItemDialog::editFinished, emodel, &OrderItemEditorModel::appendRow);
}

void OrderDialog::on_simpanButton_clicked()
{
}
