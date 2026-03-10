#include "orderdialog.h"
#include "ui_orderdialog.h"

#include "src/utils/helper.h"
#include "src/customs/flexibledelegate.h"
#include "src/dialogs/konsumenpickerdialog.h"
#include "src/dialogs/orderitemdialog.h"
#include <QHeaderView>
#include <QTimer>
#include <QAction>
#include <QMenu>
#include <QSqlTableModel>
#include <QMessageBox>

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
      {9, "use_area"},
      {10, "sale_price"},
      {11, "base_price"},
      {12, "discount_percentage"},
      {13, "discount_amount"},
      {14, "subtotal"},
      {15, "notes"},
      {16, "created_at"},
      {17, "updated_at"},
  
  
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
  void disableSignalAndSet(std::variant<QLineEdit*, QSpinBox*, QDoubleSpinBox*, QPlainTextEdit*> editor, const QVariant &value) {
    std::visit([&value](auto* editor) {
            using T = std::decay_t<decltype(*editor)>;
            editor->blockSignals(true);
            if constexpr (std::is_same_v<T, QLineEdit>)
                editor->setText(value.toString());
            else if constexpr (std::is_same_v<T, QDoubleSpinBox>)
                editor->setValue(value.toDouble());
            else if constexpr (std::is_same_v<T, QSpinBox>)
                editor->setValue(value.toInt());
            else if constexpr (std::is_same_v<T, QPlainTextEdit>)
                editor->setPlainText(value.toString());
            editor->blockSignals(false);
        }, editor);
  }
}

OrderDialog::OrderDialog(QWidget *p) : ui(new Ui::OrderDialog), emodel(new OrderItemEditorModel(this)), FormDialog(p)
{
  ui->setupUi(this);

  ui->orderItemView->setModel(emodel);

  QList<int> hiddenColumns = {0, 1, 2, 4, 10, 11, 12, 14, 15, 16};
  QList<int> numberColumns = {5, 9, 13};
  for (auto col : hiddenColumns)
  {
    ui->orderItemView->hideColumn(col);
  }
  ui->orderItemView->horizontalHeader()->setStretchLastSection(true);

  ui->orderItemView->setItemDelegateForColumn(2, new ProductDelegate(this));
  ui->orderItemView->addAction(ui->tambahItem);

  ui->orderNumberLineEdit->setText(OrderManager::generateOrderNumber());
  ui->tOrderDateTimeEdit->setDateTime(QDateTime::currentDateTime());
  ui->dLineDateTimeEdit->setDateTime(QDateTime::currentDateTime().addDays(1));

  auto numberDelegate = FlexibleDelegate::create(
      {.displayer = [](const QVariant &value, const QLocale &locale)
       { return QString("%L1").arg(value.toInt()); },
       .styler = [numberColumns](QStyleOptionViewItem &option, const QModelIndex &index)
       {
        if (index.column() == 5) { // Kolom quantity
            option.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
        } else if (numberColumns.contains(index.column())) { // Kolom sale_price dan base_price
            option.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
        }
        option.locale = QLocale(QLocale::Indonesian, QLocale::Indonesia); },},
      ui->orderItemView);
  // set delegate untuk kolom quantity, sale_price, base_price
   numberDelegate->setCreator([](QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) -> QWidget *
                             {
    auto editor = new QSpinBox(parent);
    editor->setFrame(false);
    editor->setMinimum(0);
    editor->setMaximum(9999999);
    editor->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    editor->setGroupSeparatorShown(true);
    return editor; });


  for (int col : numberColumns) {
    ui->orderItemView->setItemDelegateForColumn(col, numberDelegate);
  }

  auto unitDelegate = FlexibleDelegate::create(
      {.displayer = [](const QVariant &value, const QLocale &locale)
       { return QString("%L1").arg(value.toString()); },
       .styler = [](QStyleOptionViewItem &option, const QModelIndex &index)
       {
        option.displayAlignment = Qt::AlignHCenter | Qt::AlignVCenter;
        option.locale = QLocale(); },
       .creator = [](QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) -> QWidget * {
        auto le = new QLineEdit(parent);
        le->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        return le;
       }},
      ui->orderItemView
  );

  auto sizeDelegate = FlexibleDelegate::create(
    { .displayer = [](const QVariant &value, const QLocale &locale)
      { return QString("%L1").arg(value.toDouble()); },
      .styler = [](QStyleOptionViewItem &option, const QModelIndex &index) {
        option.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
        option.locale = QLocale();
      },
      .creator = [](QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) -> QWidget * {
        auto ds = new QDoubleSpinBox(parent);
        ds->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ds->setGroupSeparatorShown(true);
        ds->setMaximum(9999999);
        return ds;
      }
    }, ui->orderItemView
  );

  ui->orderItemView->setItemDelegateForColumn(7, sizeDelegate);
  ui->orderItemView->setItemDelegateForColumn(8, sizeDelegate);
  ui->orderItemView->setItemDelegateForColumn(6, unitDelegate);
  connect(emodel, &OrderItemEditorModel::subtotalChanged, this, &OrderDialog::updateCalculation);
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

bool OrderDialog::onSave(const QVariantMap &mp) {

  BaseManager::connection.transaction();
  
  auto opt_order = oman.create(mp);

  if (opt_order.has_value()) {
    auto ord_rec = *opt_order;
    
  }

  return false;
}

void OrderDialog::onPrepareCreate()
{
  ui->orderNumberLineEdit->setText(OrderManager::generateOrderNumber());
  ui->statusLineEdit->setText("Pending");
  ui->prioritasLineEdit->setText("Normal");
}

void OrderDialog::onPrepareModify(const QSqlRecord &orderRecord)
{
  emodel->loadFromOrder(orderRecord.value("id").toInt());
}

void OrderDialog::onOrderItemDialogAccepted()
{
  OrderItemDialog *editor = qobject_cast<OrderItemDialog *>(sender());
  QVariantMap itemData = editor->getFieldData();
  qDebug() << itemData;
  emodel->appendRow(itemData);
  editor->resetForm();
}

void OrderDialog::on_diskonDoubleSpinBox_valueChanged(double percent)
{
    if (ui->subtotalSpinBox->value() <= 0) {
        ui->diskonRpSpinBox->setValue(0);
        return;
    }

    double subtotal = ui->subtotalSpinBox->value();
    
    // Hitung diskon nominal (exact)
    double disc_exact = subtotal * percent / 100.0;
    
    // Bulatkan KE ATAS ke kelipatan 100
    int disc_amount = static_cast<int>(std::ceil(disc_exact / 100.0)) * 100;

    // Update diskon rupiah tanpa memicu signal loop
    disableSignalAndSet(ui->diskonRpSpinBox, disc_amount);

    updateCalculation();
}

void OrderDialog::on_diskonRpSpinBox_valueChanged(int disc_rupiah)
{
    double subtotal = ui->subtotalSpinBox->value();
    if (subtotal <= 0) {
        disableSignalAndSet(ui->diskonDoubleSpinBox, 0.0);
        updateCalculation();
        return;
    }

    // Hitung persentase secara akurat (tanpa pembulatan paksa)
    double percent = (static_cast<double>(disc_rupiah) / subtotal) * 100.0;

    // Optional: bulatkan persen ke 2 desimal jika diinginkan
    percent = qRound(percent * 100.0) / 100.0;

    disableSignalAndSet(ui->diskonDoubleSpinBox, percent);

    updateCalculation();
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
    customerSet.id = record.value("id").toInt();
    customerSet.name = record.value("nama_lengkap").toString();
    // simpan price level untuk digunakan di OrderItemDialog
    int priceLevelId = record.value("pl_id").toInt();
    // qDebug() << "Selected price level ID:" << priceLevelId;
    ui->priceLevelComboBox->setLevelID(priceLevelId); });
  dialog->open();
}

void OrderDialog::updateCalculation()
{
    double subtotal = emodel->calculateSubtotal();
    ui->subtotalSpinBox->setValue(subtotal);

    double diskon = ui->diskonRpSpinBox->value();
    double pajak  = ui->pajakRpSpinBox->value();

    double total = subtotal - diskon - pajak;
    ui->totalSpinBox->setValue(total);
}

void OrderDialog::on_orderItemView_customContextMenuRequested(const QPoint &pos)
{
  QMenu contextMenu;
  contextMenu.addAction(ui->tambahItem);
  if(ui->orderItemView->indexAt(pos).isValid())
  {
    contextMenu.addSeparator();
    contextMenu.addAction("Hapus Item", [this, pos]()
                          {
      QModelIndex index = ui->orderItemView->indexAt(pos);
      if (index.isValid()) {
          emodel->removeRow(index.row());
      }
    });
  }
  contextMenu.addSeparator();
  contextMenu.addAction("Resize Kolom", [this](){ui->orderItemView->resizeColumnsToContents();});
  contextMenu.exec(ui->orderItemView->viewport()->mapToGlobal(pos));
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
  connect(editor, &OrderItemDialog::editFinished, this, &OrderDialog::onOrderItemDialogAccepted);
}

void OrderDialog::on_simpanButton_clicked()
{
  if(ui->konsumenLineEdit->text().isEmpty()) {
    ui->konsumenLineEdit->setStyleSheet("background-color: rgb(255, 200, 250);");
    QMessageBox::warning(this, "Peringatan", "Nama konsumen tidak boleh kosong");
    ui->konsumenLineEdit->setFocus(Qt::ActiveWindowFocusReason);
    QTimer::singleShot(500, [this]{ui->konsumenLineEdit->setStyleSheet("");});
    return;
  }
  if(mode() == FormMode::Create) {
    accept();
  }
}

QVariantMap OrderDialog::collect() const {
  auto def = FormDialog::collect();
  // add anythink else
  if (ui->konsumenLineEdit->text() == customerSet.name) {
    def["customer_id"] = customerSet.id;
  }
  return def;
}

void OrderDialog::on_pajakRpSpinBox_valueChanged(int ch) {
  Q_UNUSED(ch);
  updateCalculation();
}