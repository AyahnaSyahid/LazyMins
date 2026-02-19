#include "advancedqueryview.h"

#include <QApplication>
#include <QMouseEvent>
#include <QMessageBox>
#include <QScrollBar>
#include <QSizePolicy>
#include <QFrame>
#include <QStyle>
#include <QPalette>
#include <QFont>
#include <QAbstractItemModel>

// ============================================================
//  AdvancedRowDelegate
// ============================================================

AdvancedRowDelegate::AdvancedRowDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

void AdvancedRowDelegate::paint(QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    if (!index.isValid()) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    // --- Tentukan warna latar baris ---
    int status = index.data(AdvancedQueryModel::RowStatusRole).toInt();
    QColor bg;
    switch (static_cast<AdvancedQueryModel::RowStatus>(status)) {
    case AdvancedQueryModel::NewRow:      bg = colorNewRow;      break;
    case AdvancedQueryModel::ModifiedRow: bg = colorModifiedRow; break;
    case AdvancedQueryModel::DeletedRow:  bg = colorDeletedRow;  break;
    default:                              bg = colorNormalRow;   break;
    }

    // Baris terpilih: blend warna selection di atas warna status
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    if (opt.state & QStyle::State_Selected) {
        // Campurkan 50% selection color + 50% status color
        QColor sel = opt.palette.highlight().color();
        bg = QColor(
            (bg.red()   + sel.red())   / 2,
            (bg.green() + sel.green()) / 2,
            (bg.blue()  + sel.blue())  / 2
        );
    }
    painter->fillRect(opt.rect, bg);

    // --- Gambar dot oranye di kiri sel pertama (kolom 0) jika dirty ---
    if (index.column() == 0 &&
        (status == AdvancedQueryModel::ModifiedRow ||
         status == AdvancedQueryModel::NewRow      ||
         status == AdvancedQueryModel::DeletedRow)) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setBrush(colorDirtyDot);
        painter->setPen(Qt::NoPen);
        int dotSize = 6;
        int x = opt.rect.left() + 3;
        int y = opt.rect.top() + (opt.rect.height() - dotSize) / 2;
        painter->drawEllipse(x, y, dotSize, dotSize);
        painter->restore();
    }

    // --- Teks: coretan untuk baris deleted ---
    if (status == AdvancedQueryModel::DeletedRow) {
        QFont f = opt.font;
        f.setStrikeOut(true);
        opt.font = f;
        // Redupkan teks
        opt.palette.setColor(QPalette::Text,
                             opt.palette.text().color().lighter(170));
    }

    // --- Hapus background dari opt agar tidak menimpa warna kita ---
    opt.backgroundBrush = QBrush(bg);

    // Gambar konten (teks, checkbox, dll) tanpa background ulang
    // Kita set State_Selected = false supaya QStyle tidak timpa bg kita,
    // tapi text-color selection tetap terjaga untuk baris terpilih
    if (option.state & QStyle::State_Selected) {
        opt.palette.setColor(QPalette::Text,
                             opt.palette.highlightedText().color());
    }
    opt.state &= ~QStyle::State_Selected; // biarkan bg kita yang menang
    QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);
}

// ============================================================
//  SortIndicatorHeader
// ============================================================

SortIndicatorHeader::SortIndicatorHeader(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
{
    setSectionsClickable(true);
    setHighlightSections(true);
}

void SortIndicatorHeader::setSortModel(AdvancedQueryModel *model)
{
    m_sortModel = model;
}

void SortIndicatorHeader::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_sortModel) {
        int logical = logicalIndexAt(event->pos());
        if (logical >= 0) {
            AdvancedQueryModel::SortInfo current = m_sortModel->currentSort();
            if (current.enabled && current.column == logical) {
                // Toggle arah sort
                Qt::SortOrder newOrder = (current.order == Qt::AscendingOrder)
                                         ? Qt::DescendingOrder
                                         : Qt::AscendingOrder;
                m_sortModel->setSort(logical, newOrder);
            } else {
                m_sortModel->setSort(logical, Qt::AscendingOrder);
            }
            viewport()->update();
        }
    }
    QHeaderView::mousePressEvent(event);
}

void SortIndicatorHeader::paintSection(QPainter *painter,
                                       const QRect &rect,
                                       int logicalIndex) const
{
    // Gambar section standar dulu
    QHeaderView::paintSection(painter, rect, logicalIndex);

    if (!m_sortModel) return;

    AdvancedQueryModel::SortInfo si = m_sortModel->currentSort();
    if (!si.enabled || si.column != logicalIndex) return;

    // Gambar segitiga sort di pojok kanan section
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(palette().text().color());

    int arrowSize = 6;
    int margin    = 4;
    int x = rect.right() - arrowSize - margin;
    int cy = rect.center().y();

    QPolygon arrow;
    if (si.order == Qt::AscendingOrder) {
        arrow << QPoint(x, cy + arrowSize / 2)
              << QPoint(x + arrowSize, cy + arrowSize / 2)
              << QPoint(x + arrowSize / 2, cy - arrowSize / 2);
    } else {
        arrow << QPoint(x, cy - arrowSize / 2)
              << QPoint(x + arrowSize, cy - arrowSize / 2)
              << QPoint(x + arrowSize / 2, cy + arrowSize / 2);
    }
    painter->drawPolygon(arrow);
    painter->restore();
}

// ============================================================
//  AdvancedQueryView — konstruktor & buildUi
// ============================================================

AdvancedQueryView::AdvancedQueryView(QWidget *parent)
    : QWidget(parent)
{
    buildUi();

    // Debounce timer untuk filter — terapkan 400ms setelah ketikan terakhir
    m_filterTimer = new QTimer(this);
    m_filterTimer->setSingleShot(true);
    m_filterTimer->setInterval(400);
    connect(m_filterTimer, &QTimer::timeout, this, &AdvancedQueryView::applyFilter);
}

void AdvancedQueryView::buildUi()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    buildToolbar();
    buildFilterBar();
    buildTableArea();
    buildPagingBar();
    buildStatusBar();
}

void AdvancedQueryView::buildToolbar()
{
    m_toolbar = new QToolBar(this);
    m_toolbar->setIconSize(QSize(20, 20));
    m_toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    // Insert row
    m_actInsert = new QAction(
        QApplication::style()->standardIcon(QStyle::SP_FileIcon),
        tr("Tambah Baris"), this);
    m_actInsert->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));
    m_actInsert->setToolTip(tr("Tambah baris baru (Ctrl++)"));
    m_toolbar->addAction(m_actInsert);

    // Delete row
    m_actDelete = new QAction(
        QApplication::style()->standardIcon(QStyle::SP_TrashIcon),
        tr("Hapus"), this);
    m_actDelete->setShortcut(QKeySequence::Delete);
    m_actDelete->setToolTip(tr("Hapus baris terpilih (Del)"));
    m_actDelete->setEnabled(false);
    m_toolbar->addAction(m_actDelete);

    m_toolbar->addSeparator();

    // Submit
    m_actSubmit = new QAction(
        QApplication::style()->standardIcon(QStyle::SP_DialogSaveButton),
        tr("Simpan"), this);
    m_actSubmit->setShortcut(QKeySequence::Save);
    m_actSubmit->setToolTip(tr("Kirim semua perubahan ke database (Ctrl+S)"));
    m_actSubmit->setEnabled(false);
    m_toolbar->addAction(m_actSubmit);

    // Revert
    m_actRevert = new QAction(
        QApplication::style()->standardIcon(QStyle::SP_DialogDiscardButton),
        tr("Batalkan"), this);
    m_actRevert->setToolTip(tr("Batalkan semua perubahan"));
    m_actRevert->setEnabled(false);
    m_toolbar->addAction(m_actRevert);

    m_toolbar->addSeparator();

    // Refresh
    m_actRefresh = new QAction(
        QApplication::style()->standardIcon(QStyle::SP_BrowserReload),
        tr("Refresh"), this);
    m_actRefresh->setShortcut(QKeySequence::Refresh);
    m_actRefresh->setToolTip(tr("Muat ulang data (F5)"));
    m_toolbar->addAction(m_actRefresh);

    m_mainLayout->addWidget(m_toolbar);
}

void AdvancedQueryView::buildFilterBar()
{
    m_filterBar = new QWidget(this);
    auto *hl = new QHBoxLayout(m_filterBar);
    hl->setContentsMargins(6, 4, 6, 4);

    auto *lblFilter = new QLabel(tr("🔍 Filter SQL:"), m_filterBar);
    hl->addWidget(lblFilter);

    m_filterEdit = new QLineEdit(m_filterBar);
    m_filterEdit->setPlaceholderText(
        tr("Contoh: dept = 'Engineering'  atau  salary > 50000"));
    m_filterEdit->setClearButtonEnabled(true);
    hl->addWidget(m_filterEdit, 1);

    m_btnApply = new QPushButton(tr("Terapkan"), m_filterBar);
    m_btnApply->setFixedWidth(90);
    hl->addWidget(m_btnApply);

    m_btnClearFilter = new QPushButton(tr("Bersihkan"), m_filterBar);
    m_btnClearFilter->setFixedWidth(90);
    hl->addWidget(m_btnClearFilter);

    // Garis pemisah di bawah filter bar
    auto *sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);

    m_mainLayout->addWidget(m_filterBar);
    m_mainLayout->addWidget(sep);
}

void AdvancedQueryView::buildTableArea()
{
    m_tableView = new QTableView(this);

    // Header kustom dengan sort indicator
    m_header = new SortIndicatorHeader(Qt::Horizontal, m_tableView);
    m_tableView->setHorizontalHeader(m_header);

    // Delegate kustom untuk warna baris
    m_delegate = new AdvancedRowDelegate(m_tableView);
    m_tableView->setItemDelegate(m_delegate);

    // Penampilan tabel
    m_tableView->setAlternatingRowColors(false); // kita atur sendiri via delegate
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_tableView->setSortingEnabled(false); // sort dihandle model, bukan proxy
    m_tableView->setEditTriggers(
        QAbstractItemView::DoubleClicked |
        QAbstractItemView::EditKeyPressed |
        QAbstractItemView::AnyKeyPressed);
    m_tableView->setShowGrid(true);
    m_tableView->setGridStyle(Qt::SolidLine);
    m_tableView->verticalHeader()->setDefaultSectionSize(24);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->horizontalHeader()->setMinimumSectionSize(60);

    // Konteks menu bawaan Qt (copy, dsb.)
    m_tableView->setContextMenuPolicy(Qt::ActionsContextMenu);

    m_mainLayout->addWidget(m_tableView, 1);

    // Garis pemisah di atas paging bar
    auto *sep = new QFrame(this);
    sep->setFrameShape(QFrame::HLine);
    sep->setFrameShadow(QFrame::Sunken);
    m_mainLayout->addWidget(sep);
}

void AdvancedQueryView::buildPagingBar()
{
    m_pagingBar = new QWidget(this);
    auto *hl = new QHBoxLayout(m_pagingBar);
    hl->setContentsMargins(6, 3, 6, 3);

    // Navigasi halaman
    m_btnFirst = new QPushButton(tr("◀◀"), m_pagingBar);
    m_btnFirst->setFixedWidth(32);
    m_btnFirst->setToolTip(tr("Halaman pertama"));
    hl->addWidget(m_btnFirst);

    m_btnPrev = new QPushButton(tr("◀"), m_pagingBar);
    m_btnPrev->setFixedWidth(32);
    m_btnPrev->setToolTip(tr("Halaman sebelumnya"));
    hl->addWidget(m_btnPrev);

    m_spinPage = new QSpinBox(m_pagingBar);
    m_spinPage->setMinimum(1);
    m_spinPage->setMaximum(1);
    m_spinPage->setFixedWidth(60);
    m_spinPage->setToolTip(tr("Nomor halaman (bisa diketik langsung)"));
    hl->addWidget(m_spinPage);

    m_lblTotalPages = new QLabel(tr("/ 1"), m_pagingBar);
    m_lblTotalPages->setFixedWidth(50);
    hl->addWidget(m_lblTotalPages);

    m_btnNext = new QPushButton(tr("▶"), m_pagingBar);
    m_btnNext->setFixedWidth(32);
    m_btnNext->setToolTip(tr("Halaman berikutnya"));
    hl->addWidget(m_btnNext);

    m_btnLast = new QPushButton(tr("▶▶"), m_pagingBar);
    m_btnLast->setFixedWidth(32);
    m_btnLast->setToolTip(tr("Halaman terakhir"));
    hl->addWidget(m_btnLast);

    hl->addSpacing(16);

    // Ukuran halaman
    auto *lblSize = new QLabel(tr("Baris/hal:"), m_pagingBar);
    hl->addWidget(lblSize);

    m_cmbPageSize = new QComboBox(m_pagingBar);
    m_cmbPageSize->addItems({"10", "20", "50", "100", "250"});
    m_cmbPageSize->setCurrentText("20");
    m_cmbPageSize->setFixedWidth(70);
    m_cmbPageSize->setToolTip(tr("Jumlah baris per halaman"));
    hl->addWidget(m_cmbPageSize);

    hl->addStretch();

    // Info ringkas
    m_lblPageInfo = new QLabel(m_pagingBar);
    m_lblPageInfo->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    hl->addWidget(m_lblPageInfo);

    m_mainLayout->addWidget(m_pagingBar);
}

void AdvancedQueryView::buildStatusBar()
{
    m_statusBar = new QStatusBar(this);
    m_statusBar->setSizeGripEnabled(false);

    m_lblStatus = new QLabel(tr("Siap"), m_statusBar);
    m_statusBar->addWidget(m_lblStatus, 1);

    m_lblPending = new QLabel(m_statusBar);
    m_lblPending->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_statusBar->addPermanentWidget(m_lblPending);

    m_lblError = new QLabel(m_statusBar);
    m_lblError->setStyleSheet("color: red; font-weight: bold;");
    m_lblError->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_statusBar->addPermanentWidget(m_lblError);

    m_mainLayout->addWidget(m_statusBar);
}

// ============================================================
//  setModel
// ============================================================

void AdvancedQueryView::setModel(AdvancedQueryModel *model)
{
    // Lepas koneksi lama
    if (m_model) {
        disconnect(m_model, nullptr, this, nullptr);
    }

    m_model = model;
    m_tableView->setModel(model);
    m_header->setSortModel(model);

    // Tambahkan action delete ke context menu tabel
    m_tableView->addAction(m_actInsert);
    m_tableView->addAction(m_actDelete);

    connectSignals();
    updatePagingBar();
    updateStatusBar();
    updateToolbarState();
}

AdvancedQueryModel *AdvancedQueryView::queryModel() const { return m_model; }
QTableView        *AdvancedQueryView::tableView()   const { return m_tableView; }

// ============================================================
//  connectSignals
// ============================================================

void AdvancedQueryView::connectSignals()
{
    // Toolbar
    connect(m_actInsert,  &QAction::triggered, this, &AdvancedQueryView::onInsertRow);
    connect(m_actDelete,  &QAction::triggered, this, &AdvancedQueryView::onDeleteSelectedRows);
    connect(m_actSubmit,  &QAction::triggered, this, &AdvancedQueryView::onSubmitAll);
    connect(m_actRevert,  &QAction::triggered, this, &AdvancedQueryView::onRevertAll);
    connect(m_actRefresh, &QAction::triggered, this, &AdvancedQueryView::refresh);

    // Filter bar
    connect(m_filterEdit,      &QLineEdit::textChanged,
            this, &AdvancedQueryView::onFilterTextChanged);
    connect(m_btnApply,        &QPushButton::clicked,
            this, &AdvancedQueryView::applyFilter);
    connect(m_btnClearFilter,  &QPushButton::clicked, this, [this]() {
        m_filterEdit->clear();
        if (m_model) {
            m_model->setFilter(QString());
            updatePagingBar();
            updateStatusBar();
        }
    });

    // Paging
    connect(m_btnFirst, &QPushButton::clicked, this, &AdvancedQueryView::onFirstPage);
    connect(m_btnPrev,  &QPushButton::clicked, this, &AdvancedQueryView::onPrevPage);
    connect(m_btnNext,  &QPushButton::clicked, this, &AdvancedQueryView::onNextPage);
    connect(m_btnLast,  &QPushButton::clicked, this, &AdvancedQueryView::onLastPage);
    connect(m_cmbPageSize, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AdvancedQueryView::onPageSizeChanged);
    connect(m_spinPage, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &AdvancedQueryView::onPageSpinChanged);

    // Seleksi berubah → update tombol delete
    connect(m_tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &AdvancedQueryView::updateToolbarState);

    // Model reset → perbarui UI
    if (m_model) {
        connect(m_model, &QAbstractItemModel::modelReset,
                this, [this]() {
                    updatePagingBar();
                    updateStatusBar();
                    updateToolbarState();
                });
        connect(m_model, &QAbstractItemModel::dataChanged,
                this, [this]() {
                    updateStatusBar();
                    updateToolbarState();
                });
        connect(m_model, &QAbstractItemModel::rowsInserted,
                this, [this]() {
                    updateStatusBar();
                    updateToolbarState();
                });
        connect(m_model, &QAbstractItemModel::rowsRemoved,
                this, [this]() {
                    updateStatusBar();
                    updateToolbarState();
                });
    }
}

// ============================================================
//  Visibilitas & konfigurasi
// ============================================================

void AdvancedQueryView::setToolbarVisible(bool v)   { m_toolbar->setVisible(v); }
void AdvancedQueryView::setFilterBarVisible(bool v) { m_filterBar->setVisible(v); }
void AdvancedQueryView::setPagingBarVisible(bool v) { m_pagingBar->setVisible(v); }
void AdvancedQueryView::setStatusBarVisible(bool v) { m_statusBar->setVisible(v); }

void AdvancedQueryView::setAllowInsert(bool allow)
{
    m_actInsert->setVisible(allow);
}

void AdvancedQueryView::setAllowDelete(bool allow)
{
    m_actDelete->setVisible(allow);
}

// ============================================================
//  refresh()
// ============================================================

void AdvancedQueryView::refresh()
{
    if (!m_model) return;
    m_model->refresh();
    updatePagingBar();
    updateStatusBar();
    updateToolbarState();
}

// ============================================================
//  Toolbar slots
// ============================================================

void AdvancedQueryView::onInsertRow()
{
    if (!m_model) return;
    int newRow = m_model->rowCount();
    m_model->insertRows(newRow, 1);
    // Scroll ke baris baru dan mulai edit kolom pertama yang bisa diedit
    QModelIndex idx = m_model->index(newRow, 1); // skip id (biasanya auto)
    m_tableView->scrollTo(idx);
    m_tableView->setCurrentIndex(idx);
    m_tableView->edit(idx);
    updateToolbarState();
    updateStatusBar();
}

void AdvancedQueryView::onDeleteSelectedRows()
{
    if (!m_model) return;

    QModelIndexList sel = m_tableView->selectionModel()->selectedRows();
    if (sel.isEmpty()) return;

    // Konfirmasi
    int ret = QMessageBox::question(
        this,
        tr("Konfirmasi Hapus"),
        tr("Hapus %1 baris yang dipilih?\n"
           "Perubahan belum tersimpan hingga Anda menekan Simpan.")
        .arg(sel.size()),
        QMessageBox::Yes | QMessageBox::No
    );
    if (ret != QMessageBox::Yes) return;

    // Hapus dari bawah ke atas agar index tidak bergeser
    QList<int> rows;
    for (const QModelIndex &idx : sel)
        rows.append(idx.row());
    std::sort(rows.begin(), rows.end(), std::greater<int>());

    for (int row : rows)
        m_model->removeRows(row, 1);

    updateToolbarState();
    updateStatusBar();
}

void AdvancedQueryView::onSubmitAll()
{
    if (!m_model) return;

    if (!m_model->hasPendingChanges()) return;

    m_lblError->clear();
    m_lblStatus->setText(tr("Menyimpan..."));

    bool ok = m_model->submitAll();
    if (ok) {
        m_lblStatus->setText(tr("✔ Semua perubahan berhasil disimpan."));
        m_lblError->clear();
        emit submitRequested();
    } else {
        QString err = m_model->lastError();
        m_lblError->setText(tr("⚠ Error: ") + err);
        m_lblStatus->setText(tr("Simpan gagal."));
        QMessageBox::critical(this, tr("Error Database"), err);
        emit errorOccurred(err);
    }

    updatePagingBar();
    updateStatusBar();
    updateToolbarState();
}

void AdvancedQueryView::onRevertAll()
{
    if (!m_model) return;
    if (!m_model->hasPendingChanges()) return;

    int ret = QMessageBox::question(
        this,
        tr("Batalkan Perubahan"),
        tr("Batalkan semua perubahan yang belum disimpan?"),
        QMessageBox::Yes | QMessageBox::No
    );
    if (ret != QMessageBox::Yes) return;

    m_model->revertAll();
    m_lblStatus->setText(tr("Semua perubahan dibatalkan."));
    m_lblError->clear();
    emit revertRequested();
    updatePagingBar();
    updateStatusBar();
    updateToolbarState();
}

// ============================================================
//  Filter slots
// ============================================================

void AdvancedQueryView::onFilterTextChanged(const QString &)
{
    // Debounce: terapkan filter setelah 400ms tidak mengetik
    m_filterTimer->start();
}

void AdvancedQueryView::applyFilter()
{
    m_filterTimer->stop();
    if (!m_model) return;

    QString text = m_filterEdit->text().trimmed();
    m_model->setFilter(text);
    updatePagingBar();
    updateStatusBar();
}

// ============================================================
//  Paging slots
// ============================================================

void AdvancedQueryView::onFirstPage()
{
    if (!m_model) return;
    m_model->setPage(1);
    updatePagingBar();
    updateStatusBar();
}

void AdvancedQueryView::onPrevPage()
{
    if (!m_model) return;
    m_model->setPage(m_model->currentPage() - 1);
    updatePagingBar();
    updateStatusBar();
}

void AdvancedQueryView::onNextPage()
{
    if (!m_model) return;
    m_model->setPage(m_model->currentPage() + 1);
    updatePagingBar();
    updateStatusBar();
}

void AdvancedQueryView::onLastPage()
{
    if (!m_model) return;
    m_model->setPage(m_model->totalPages());
    updatePagingBar();
    updateStatusBar();
}

void AdvancedQueryView::onPageSizeChanged(int /*index*/)
{
    if (!m_model) return;
    int size = m_cmbPageSize->currentText().toInt();
    if (size > 0) {
        m_model->setPageSize(size);
        updatePagingBar();
        updateStatusBar();
    }
}

void AdvancedQueryView::onPageSpinChanged(int value)
{
    if (!m_model) return;
    if (value != m_model->currentPage()) {
        m_model->setPage(value);
        updatePagingBar();
        updateStatusBar();
    }
}

// ============================================================
//  UI update helpers
// ============================================================

void AdvancedQueryView::updatePagingBar()
{
    if (!m_model) return;

    int cur   = m_model->currentPage();
    int total = m_model->totalPages();

    // Blokir valueChanged saat kita update programatik
    QSignalBlocker sb(m_spinPage);
    m_spinPage->setMaximum(qMax(1, total));
    m_spinPage->setValue(cur);
    m_lblTotalPages->setText(tr("/ %1").arg(total));

    // Aktif/nonaktifkan tombol navigasi
    m_btnFirst->setEnabled(cur > 1);
    m_btnPrev ->setEnabled(cur > 1);
    m_btnNext ->setEnabled(cur < total);
    m_btnLast ->setEnabled(cur < total);

    // Info ringkas: "Menampilkan 1–20 dari 250 record"
    int pageSize = m_model->pageSize();
    int totalRec = m_model->totalRecords();
    int firstRec = (cur - 1) * pageSize + 1;
    int lastRec  = qMin(cur * pageSize, totalRec);
    if (totalRec == 0) {
        m_lblPageInfo->setText(tr("Tidak ada data"));
    } else {
        m_lblPageInfo->setText(
            tr("Menampilkan %1 – %2 dari %3 record")
            .arg(firstRec).arg(lastRec).arg(totalRec)
        );
    }

    // Sinkronkan combobox page size tanpa trigger event
    {
        QSignalBlocker sb2(m_cmbPageSize);
        m_cmbPageSize->setCurrentText(QString::number(m_model->pageSize()));
    }
}

void AdvancedQueryView::updateStatusBar()
{
    if (!m_model) {
        m_lblStatus->setText(tr("Tidak ada model."));
        m_lblPending->clear();
        return;
    }

    // Filter aktif?
    QString filterInfo;
    if (!m_model->currentFilter().isEmpty())
        filterInfo = tr("  [Filter: %1]").arg(m_model->currentFilter());

    // Sort aktif?
    QString sortInfo;
    AdvancedQueryModel::SortInfo si = m_model->currentSort();
    if (si.enabled) {
        sortInfo = tr("  [Sort: kolom %1 %2]")
                   .arg(si.column)
                   .arg(si.order == Qt::AscendingOrder ? "↑" : "↓");
    }

    m_lblStatus->setText(tr("Halaman %1/%2 · %3 record%4%5")
                         .arg(m_model->currentPage())
                         .arg(m_model->totalPages())
                         .arg(m_model->totalRecords())
                         .arg(filterInfo)
                         .arg(sortInfo));

    // Indikator pending
    if (m_model->hasPendingChanges()) {
        m_lblPending->setText(tr("● Ada perubahan belum disimpan"));
        m_lblPending->setStyleSheet("color: #E65100; font-weight: bold;");
    } else {
        m_lblPending->setText(tr("✔ Tersimpan"));
        m_lblPending->setStyleSheet("color: green;");
    }
}

void AdvancedQueryView::updateToolbarState()
{
    bool hasModel   = (m_model != nullptr);
    bool hasPending = hasModel && m_model->hasPendingChanges();
    bool hasSelect  = hasModel &&
                      m_tableView->selectionModel() &&
                      !m_tableView->selectionModel()->selectedRows().isEmpty();

    m_actInsert ->setEnabled(hasModel);
    m_actDelete ->setEnabled(hasModel && hasSelect);
    m_actSubmit ->setEnabled(hasPending);
    m_actRevert ->setEnabled(hasPending);
    m_actRefresh->setEnabled(hasModel);
}


