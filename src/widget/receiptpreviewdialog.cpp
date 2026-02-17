#include "receiptpreviewdialog.h"
#include "ui_receiptpreviewdialog.h"
#include "../invoicedatatype.h"
#include "../databaseinterface.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QRegularExpression>
#include <QFont>
#include <QPen>
#include <QGraphicsSimpleTextItem>
#include <QLocale>
#include <QRectF>

namespace {
  std::optional<PrintInvoiceParams> getParam(qlonglong inv) {
    return DatabaseInterface::instance().getPrintInvoiceParams(inv);
  }

  QString wrapText(const QString &text, int maxChars) {
    if (maxChars <= 0) return text;

    // Split berdasarkan spasi DAN newline untuk mendapatkan semua kata
    // Qt::SkipEmptyParts akan otomatis membuang spasi ganda dan newline kosong
    QStringList words = text.split(QRegularExpression("[\\s\\n]+"), Qt::SkipEmptyParts);

    QStringList result;
    QString currentLine;

    for (QString word : words) {
      // Kasus 1: Kata sangat panjang (Potong Paksa)
      if (word.length() > maxChars) {
        if (!currentLine.isEmpty()) {
            result.append(currentLine);
            currentLine.clear();
        }
        
        while (word.length() > maxChars) {
            result.append(word.left(maxChars));
            word = word.mid(maxChars);
        }
        currentLine = word; 
      } 
      // Kasus 2: Kata muat di baris saat ini
      else if (currentLine.isEmpty() || (currentLine.length() + 1 + word.length() <= maxChars)) {
        if (!currentLine.isEmpty()) currentLine += " ";
        currentLine += word;
      } 
      // Kasus 3: Baris penuh, buat baris baru
      else {
        result.append(currentLine);
        currentLine = word;
      }
    }

    if (!currentLine.isEmpty()) result.append(currentLine);
    return result.join("\n").trimmed();
  }
  
  void drawSingleItem(QGraphicsScene *s, const InvoiceData::ItemData &i, QRectF *rf, const QFont &f) {
    auto gName = s->addSimpleText(wrapText(i.productName, 22), f);
    gName->setPos(rf->bottomLeft());
    auto rect1 = gName->sceneBoundingRect();
    auto qtyPrice = QString("%L1@%L2").arg(i.unitQty, 4).arg(i.unitPrice, 9);
    auto subTotal = QString("%L2").arg(i.subTotal);
    auto gQtyPrice = s->addSimpleText(qtyPrice, f);
    gQtyPrice->setPos(rect1.bottomLeft());
    rect1 = gQtyPrice->sceneBoundingRect();
    QFont bold(f);
    bold.setBold(true);
    auto gSubTotal = s->addSimpleText(QString("%L1").arg(i.subTotal), bold);
    gSubTotal->setPos(rect1.topLeft());
    rect1 = gSubTotal->sceneBoundingRect();
    rect1.moveRight(rf->right());
    gSubTotal->setPos(rect1.topLeft());
    rf->setBottom(rect1.bottom());
  }
  
  void drawItems(QGraphicsScene *s, const QList<InvoiceData::ItemData> &itemList, QRectF *upperRect, const QFont& f) {
    for(const auto &item : itemList) {
      drawSingleItem(s, item, upperRect, f);
    }
  }
  
  
}

ReceiptPreviewDialog::ReceiptPreviewDialog(qlonglong inv, QWidget *parent)
: ui(new Ui::ReceiptPreviewDialog), scene(new QGraphicsScene), m_ready(false), param(*getParam(inv)), QDialog(parent)
{
  ui->setupUi(this);
  ui->rView->setScene(scene);
  auto &di = DatabaseInterface::instance();
  m_ready = !param.adminName.isEmpty();
  draw();
  ui->rView->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
  ui->rView->setMinimumSize(scene->sceneRect().size().toSize());
  adjustSize();
  setFixedSize(geometry().size());
}

ReceiptPreviewDialog::~ReceiptPreviewDialog() { delete ui; }

void ReceiptPreviewDialog::draw()
{
  scene->clear();
  if (!m_ready) return ;
  QFont normalFont("Courier New", 9),
        boldFont("Courier New", 9),
        bigFont("Courier New", 14);
  QString eqLine(33, QChar('=')), dashLine(33, QChar('-'));
  boldFont.setBold(true);
  bigFont.setBold(true);
  auto g1 = scene->addSimpleText(eqLine, normalFont);
  auto r1 = g1->sceneBoundingRect();
  auto cx = r1.center().x();
  auto g2 = scene->addSimpleText(param.storeInfo.storeName, bigFont);
  g2->setPos( cx - g2->sceneBoundingRect().width() / 2.0, r1.bottom() );
  g1 = scene->addSimpleText(param.storeInfo.storeAddr, normalFont);
  g1->setPos( cx - g1->sceneBoundingRect().width() / 2.0, g2->sceneBoundingRect().bottom() );
  auto lastRect = g1->sceneBoundingRect();
  g1 = scene->addSimpleText(param.storeInfo.storePhone, boldFont);
  g1->setPos( cx - g1->sceneBoundingRect().width() / 2.0, lastRect.bottom());
  lastRect = g1->sceneBoundingRect();
  g1 = scene->addSimpleText(eqLine, normalFont);
  g1->setY(lastRect.bottom());
  lastRect = g1->sceneBoundingRect();
  g1 = scene->addSimpleText(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"), boldFont);
  g1->setPos(cx - g1->sceneBoundingRect().width() / 2.0, lastRect.bottom());
  lastRect = g1->sceneBoundingRect();
  g1 = scene->addSimpleText(dashLine, normalFont);
  g1->setY(lastRect.bottom());
  lastRect = g1->sceneBoundingRect();
  // DrawInvoice Info
  g1 = scene->addSimpleText(QString("Tanggal  : %1").arg(param.invoiceDate), normalFont);
  g1->setY(lastRect.bottom());
  g2 = scene->addSimpleText(QString("No       : %1").arg(param.invoiceCode), normalFont);
  g2->setY(g1->sceneBoundingRect().bottom());
  g1 = scene->addSimpleText(QString("Konsumen : %1").arg(param.customerName), normalFont);
  g1->setY(g2->sceneBoundingRect().bottom());
  g2 = scene->addSimpleText(QString("         - %1").arg(param.customerPhone.isEmpty() ? "N/A" : param.customerPhone), normalFont);
  g2->setY(g1->sceneBoundingRect().bottom());
  g1 = scene->addSimpleText(QString("Admin    : %1").arg(param.adminName), normalFont);
  g1->setY(g2->sceneBoundingRect().bottom());
  g2 = scene->addSimpleText(eqLine, normalFont);
  g2->setY(g1->sceneBoundingRect().bottom());
  g1 = scene->addSimpleText("  Nama Barang", boldFont);
  g1->setY(g2->sceneBoundingRect().bottom());
  g2 = scene->addSimpleText("Harga    ", boldFont);
  g2->setPos(scene->sceneRect().right() - g2->sceneBoundingRect().width(), g1->sceneBoundingRect().y());
  g1 = scene->addSimpleText(dashLine, normalFont);
  g1->setY(g2->sceneBoundingRect().bottom());
  lastRect = g1->sceneBoundingRect();
  
  drawItems(scene, param.itemList, &lastRect, normalFont);
  
  // sumarry
  qlonglong tval = 0;
  for(const auto &i : param.itemList) {
    tval += i.subTotal;
  }
  
  g1 = scene->addSimpleText(dashLine, normalFont);
  g1->setY(lastRect.bottom());
  g2 = scene->addSimpleText(QString("Total : Rp %L1").arg(tval, 16), boldFont);
  g2->setY(g1->sceneBoundingRect().bottom());
  g2->setX(g1->sceneBoundingRect().width() - g2->sceneBoundingRect().width());
  g1 = scene->addSimpleText(dashLine, normalFont);
  g1->setY(g2->sceneBoundingRect().bottom());
  scene->setSceneRect(scene->sceneRect().adjusted(-10, -20, 10, 20));
}