#pragma once

#include <QWidget>
#include <QTableView>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QToolBar>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAction>
#include <QPainter>
#include <QTimer>

#include "../models/advancedquerymodel.h"

// ============================================================
//  AdvancedRowDelegate
//  Memberi warna baris sesuai status pending dan menggambar
//  indikator dirty di tepi kiri sel.
// ============================================================
class AdvancedRowDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit AdvancedRowDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    // Warna bisa dikustomisasi dari luar
    QColor colorNewRow      { 0xC8, 0xE6, 0xC9 };   // hijau muda
    QColor colorModifiedRow { 0xFF, 0xF9, 0xC4 };   // kuning muda
    QColor colorDeletedRow  { 0xFF, 0xCC, 0xBC };   // merah muda
    QColor colorNormalRow   { Qt::white };
    QColor colorDirtyDot    { 0xF5, 0x75, 0x10 };   // oranye, dot perubahan
};

// ============================================================
//  SortIndicatorHeader
//  Header horizontal yang menampilkan ikon sort dan
//  meneruskan klik ke AdvancedQueryModel::setSort().
// ============================================================
class SortIndicatorHeader : public QHeaderView
{
    Q_OBJECT
public:
    explicit SortIndicatorHeader(Qt::Orientation orientation, QWidget *parent = nullptr);

    // Model harus di-set via setSortModel()
    void setSortModel(AdvancedQueryModel *model);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override;

private:
    AdvancedQueryModel *m_sortModel { nullptr };
};

// ============================================================
//  AdvancedQueryView
//  Widget utama: toolbar + filter bar + tabel + paging bar + status bar.
// ============================================================
class AdvancedQueryView : public QWidget
{
    Q_OBJECT

public:
    explicit AdvancedQueryView(QWidget *parent = nullptr);

    // Pasang model — view akan mengambil alih interaksi pengguna
    void setModel(AdvancedQueryModel *model);
    AdvancedQueryModel *queryModel() const;

    // Akses ke inner table view (untuk kustomisasi lanjut)
    QTableView *tableView() const;

    // Tampilkan / sembunyikan bagian-bagian UI
    void setToolbarVisible(bool visible);
    void setFilterBarVisible(bool visible);
    void setPagingBarVisible(bool visible);
    void setStatusBarVisible(bool visible);

    // Konfigurasi toolbar
    void setAllowInsert(bool allow);
    void setAllowDelete(bool allow);

signals:
    void submitRequested();
    void revertRequested();
    void errorOccurred(const QString &message);

public slots:
    void refresh();

private slots:
    // Toolbar actions
    void onInsertRow();
    void onDeleteSelectedRows();
    void onSubmitAll();
    void onRevertAll();

    // Filter bar
    void onFilterTextChanged(const QString &text);
    void applyFilter();

    // Paging
    void onFirstPage();
    void onPrevPage();
    void onNextPage();
    void onLastPage();
    void onPageSizeChanged(int index);
    void onPageSpinChanged(int value);

    // Model → UI
    void updatePagingBar();
    void updateStatusBar();
    void updateToolbarState();

private:
    void buildUi();
    void buildToolbar();
    void buildFilterBar();
    void buildTableArea();
    void buildPagingBar();
    void buildStatusBar();
    void connectSignals();

    // --- Data ---
    AdvancedQueryModel *m_model { nullptr };

    // --- UI widgets ---
    QVBoxLayout      *m_mainLayout    { nullptr };

    // Toolbar
    QToolBar         *m_toolbar       { nullptr };
    QAction          *m_actInsert     { nullptr };
    QAction          *m_actDelete     { nullptr };
    QAction          *m_actSubmit     { nullptr };
    QAction          *m_actRevert     { nullptr };
    QAction          *m_actRefresh    { nullptr };

    // Filter bar
    QWidget          *m_filterBar     { nullptr };
    QLineEdit        *m_filterEdit    { nullptr };
    QPushButton      *m_btnApply      { nullptr };
    QPushButton      *m_btnClearFilter{ nullptr };
    QTimer           *m_filterTimer   { nullptr };  // debounce

    // Table
    QTableView       *m_tableView     { nullptr };
    SortIndicatorHeader *m_header     { nullptr };
    AdvancedRowDelegate *m_delegate   { nullptr };

    // Paging bar
    QWidget          *m_pagingBar     { nullptr };
    QPushButton      *m_btnFirst      { nullptr };
    QPushButton      *m_btnPrev       { nullptr };
    QPushButton      *m_btnNext       { nullptr };
    QPushButton      *m_btnLast       { nullptr };
    QSpinBox         *m_spinPage      { nullptr };
    QLabel           *m_lblTotalPages { nullptr };
    QComboBox        *m_cmbPageSize   { nullptr };
    QLabel           *m_lblPageInfo   { nullptr };

    // Status bar
    QStatusBar       *m_statusBar     { nullptr };
    QLabel           *m_lblStatus     { nullptr };
    QLabel           *m_lblPending    { nullptr };
    QLabel           *m_lblError      { nullptr };
};
