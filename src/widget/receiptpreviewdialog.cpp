#include "receiptpreviewdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFont>
#include <QPen>
#include <QGraphicsLineItem>
#include <QLocale>

ReceiptPreviewDialog::ReceiptPreviewDialog(const PrintInvoiceParams &params, QWidget *parent)
    : QDialog(parent),
      m_params(params)
{
    setWindowTitle("Pratinjau Struk");
    resize(480, 680);           // ukuran jendela awal – bisa di-resize user
    setupUi();
    populateScene();
}

ReceiptPreviewDialog::~ReceiptPreviewDialog()
{
}

void ReceiptPreviewDialog::setupUi()
{
    m_scene = new QGraphicsScene(this);
    m_view = new QGraphicsView(m_scene, this);

    m_view->setRenderHint(QPainter::TextAntialiasing, false);
    m_view->setRenderHint(QPainter::SmoothPixmapTransform, false);
    m_view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    m_view->setBackgroundBrush(QColor(245, 245, 240));  // mirip kertas thermal

    // Scene rect – lebar sesuai kertas, tinggi sementara (akan diupdate)
    m_scene->setSceneRect(0, 0, PAPER_WIDTH_PX, 2000);

    // Layout utama
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->addWidget(m_view);

    // Tombol kontrol di bawah
    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnClose = new QPushButton("Tutup", this);
    QPushButton *btnZoomIn = new QPushButton("Zoom +", this);
    QPushButton *btnZoomOut = new QPushButton("Zoom –", this);

    btnLayout->addStretch();
    btnLayout->addWidget(btnZoomIn);
    btnLayout->addWidget(btnZoomOut);
    btnLayout->addWidget(btnClose);

    mainLayout->addLayout(btnLayout);

    connect(btnClose,   &QPushButton::clicked, this, &QDialog::accept);
    connect(btnZoomIn,  &QPushButton::clicked, this, [=](){ m_view->scale(1.25, 1.25); });
    connect(btnZoomOut, &QPushButton::clicked, this, [=](){ m_view->scale(0.80, 0.80); });
}

void ReceiptPreviewDialog::populateScene()
{
    m_currentY = 10.0;

    QFont headerFont("Courier New", 12, QFont::Bold);
    QFont normalFont("Courier New", 10);
    QFont smallFont("Courier New", 9);

    headerFont.setStyleHint(QFont::TypeWriter);
    normalFont.setStyleHint(QFont::TypeWriter);
    smallFont.setStyleHint(QFont::TypeWriter);

    // ── Header ───────────────────────────────────────────────
    drawHeader();

    // ── Daftar Item ──────────────────────────────────────────
    drawItems();

    // ── Pembayaran ───────────────────────────────────────────
    drawPayments();

    // ── Footer ───────────────────────────────────────────────
    drawFooter();

    // Update tinggi scene sesuai konten
    m_scene->setSceneRect(0, 0, PAPER_WIDTH_PX, m_currentY + 60);
}

void ReceiptPreviewDialog::drawHeader()
{
    QFont titleFont("Courier New", 13, QFont::Bold);
    titleFont.setStyleHint(QFont::TypeWriter);

    addCenteredText(m_params.storeInfo.storeName, titleFont, m_currentY);
    m_currentY += 24;

    addCenteredText(m_params.storeInfo.storeAddr,  QFont("Courier New", 9), m_currentY);
    m_currentY += 16;
    addCenteredText(m_params.storeInfo.storePhone, QFont("Courier New", 9), m_currentY);
    m_currentY += 20;

    addCenteredText("====================================", QFont("Courier New", 10), m_currentY);
    m_currentY += 18;

    addLeftText("No. Invoice : " + m_params.invoiceCode, QFont("Courier New", 10), m_currentY);
    m_currentY += 16;

    addLeftText("Kasir       : " + m_params.adminName, QFont("Courier New", 10), m_currentY);
    m_currentY += 16;

    if (!m_params.customerName.isEmpty()) {
        addLeftText("Pelanggan   : " + m_params.customerName, QFont("Courier New", 10), m_currentY);
        m_currentY += 16;
    }

    addCenteredText("------------------------------------", QFont("Courier New", 10), m_currentY);
    m_currentY += 18;
}

void ReceiptPreviewDialog::drawItems()
{
    if (m_params.itemList.isEmpty()) return;

    addLeftText("Item",                  QFont("Courier New", 10, QFont::Bold), m_currentY);
    addRightText("Subtotal",            QFont("Courier New", 10, QFont::Bold), m_currentY);
    m_currentY += 20;

    QLocale locale(QLocale::Indonesian);  // untuk format Rupiah

    for (const auto &item : m_params.itemList)
    {
        QString line = QString("%1 x %2")
                           .arg(item.unitQty)
                           .arg(item.productName);

        addLeftText(line, QFont("Courier New", 10), m_currentY);

        QString priceStr = locale.toString(item.subTotal);
        addRightText("Rp " + priceStr, QFont("Courier New", 10), m_currentY);

        m_currentY += 18;
    }

    m_currentY += 8;
    drawDashedLine(m_currentY);
    m_currentY += 18;
}

void ReceiptPreviewDialog::drawPayments()
{
    if (m_params.paymentList.isEmpty()) return;

    addLeftText("Pembayaran",           QFont("Courier New", 10, QFont::Bold), m_currentY);
    addRightText("Jumlah",              QFont("Courier New", 10, QFont::Bold), m_currentY);
    m_currentY += 20;

    QLocale locale(QLocale::Indonesian);
    int totalPaid = 0;

    for (const auto &pay : m_params.paymentList)
    {
        QString method = pay.method;
        if (!pay.paymentTime.isNull()) {
            method += "  " + pay.paymentTime.toString("dd/MM/yy HH:mm");
        }

        addLeftText(method, QFont("Courier New", 10), m_currentY);

        QString amountStr = locale.toString(pay.amount);
        addRightText("Rp " + amountStr, QFont("Courier New", 10), m_currentY);

        totalPaid += pay.amount;
        m_currentY += 18;
    }

    m_currentY += 12;
    drawDashedLine(m_currentY);
    m_currentY += 18;
}

void ReceiptPreviewDialog::drawFooter()
{
    addCenteredText("====================================", QFont("Courier New", 10), m_currentY);
    m_currentY += 20;

    addCenteredText("Terima Kasih", QFont("Courier New", 11, QFont::Bold), m_currentY);
    m_currentY += 24;

    addCenteredText("Barang yang sudah dibeli tidak dapat", QFont("Courier New", 9), m_currentY);
    m_currentY += 16;
    addCenteredText("ditukar / dikembalikan",              QFont("Courier New", 9), m_currentY);
    m_currentY += 24;
}

void ReceiptPreviewDialog::drawDashedLine(double y)
{
    QPen pen(Qt::black);
    pen.setStyle(Qt::DashLine);
    pen.setWidth(1);

    double contentWidth = PAPER_WIDTH_PX - LEFT_MARGIN - RIGHT_MARGIN;
    m_scene->addLine(LEFT_MARGIN, y, LEFT_MARGIN + contentWidth, y, pen);
}

QGraphicsTextItem* ReceiptPreviewDialog::addCenteredText(const QString &text, const QFont &font, double y)
{
    auto *item = m_scene->addText(text, font);
    item->setDefaultTextColor(Qt::black);
    double x = (PAPER_WIDTH_PX - item->boundingRect().width()) / 2.0;
    item->setPos(x, y);
    return item;
}

QGraphicsTextItem* ReceiptPreviewDialog::addLeftText(const QString &text, const QFont &font, double y, double margin)
{
    auto *item = m_scene->addText(text, font);
    item->setDefaultTextColor(Qt::black);
    item->setPos(margin, y);
    return item;
}

QGraphicsTextItem* ReceiptPreviewDialog::addRightText(const QString &text, const QFont &font, double y, double margin)
{
    auto *item = m_scene->addText(text, font);
    item->setDefaultTextColor(Qt::black);
    double x = PAPER_WIDTH_PX - item->boundingRect().width() - margin;
    item->setPos(x, y);
    return item;
}