#include "receiptpreviewdialog.h"
#include "ui_receiptpreviewdialog.h"
#include "../invoicedatatype.h"
#include "../databaseinterface.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFont>
#include <QPen>
#include <QGraphicsSimpleTextItem>
#include <QLocale>
#include <QRectF>

namespace {
  PrintInvoiceParams getParam(qlonglong inv) {
    return DatabaseInterface::instance().getPrintInvoiceParams(inv);
  }

  QString wrapText(const QString &text, int maxChars) {
    if (maxChars <= 0) return text;

    // Split berdasarkan spasi DAN newline untuk mendapatkan semua kata
    // Qt::SkipEmptyParts akan otomatis membuang spasi ganda dan newline kosong
    QStringList words = text.split(QRegExp("[\s\n]+"), Qt::SkipEmptyParts);

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
    return result.join("\n");
  }
  
  void drawSingleItem(QGraphicsScene *s, const InvoiceData:ItemData &i, const QPointF &pos, const QFont &f) {
    auto gName = s->addSimpleText(wrapText(i.productName, 32), f);
    gName->setPos(pos)
    auto r1 = gName->boundingRect();
    
  }
}

ReceiptPreviewDialog::ReceiptPreviewDialog(qlonglong inv, QWidget *parent)
: ui(new Ui::ReceiptPreviewDialog), scene(new QGraphicsScene), m_ready(false), param(getParam(inv)), QDialog(parent)
{
  ui->setupUi(this);
  ui->rView->setScene(scene);
  auto &di = DatabaseInterface::instance();
  m_ready = !param.adminName.isEmpty();
  draw();
}

ReceiptPreviewDialog::~ReceiptPreviewDialog() { delete ui; }

void ReceiptPreviewDialog::draw()
{
  scene->clear();
  if (!m_ready) return ;
  QFont normalFont("Courier New", 9),
        boldFont("Courier New", 9),
        bigFont("Courier New", 14);
  QString eqLine(32, QChar('=')), dashLine(32, QChar('-'));
  boldFont.setBold(true);
  bigFont.setBold(true);
  QRectF sr(1,1,1,1);
  auto g1 = scene->addSimpleText(eqLine, normalFont);
  auto g2 = scene->addSimpleText(param.storeInfo.storeName, bigFont);
  g2->setY(g1->boundingRect().bottomLeft().y());
  auto r1 = g1->boundingRect();
  sr.setTopLeft(r1.topLeft());
  sr.setRight(r1.right());
  auto r2 = g2->boundingRect();
  r2.moveCenter(r1.center());
  r2.moveTop(r1.bottom());
  g2->setPos(r2.topLeft());
  g1 = scene->addSimpleText(param.storeInfo.storeAddr, normalFont);
  r1 = g1->boundingRect();
  r1.moveCenter(r2.center());
  r1.moveTop(r2.bottom());
  g1->setPos(r1.topLeft());
  g2 = scene->addSimpleText(param.storeInfo.storePhone, normalFont);
  r2 = g2->boundingRect();
  r2.moveCenter(r1.center());
  r2.moveTop(r1.bottom());
  g2->setPos(r2.topLeft());
  g1 = scene->addSimpleText(eqLine, normalFont);
  r1 = g1->boundingRect();
  r1.moveCenter(r2.center());
  r1.moveTop(r2.bottom());
  g1->setPos(r1.topLeft());
  g2 = scene->addSimpleText("   Nama Barang", normalFont);
  r2 = g2->boundingRect();
  r2.setTopLeft(r1.bottomLeft());
  g2->setPos(r2.topLeft());
  g1 = scene->addSimpleText("Harga    ", normalFont);
  r1 = g1->boundingRect();
  r1.moveTop(r2.top());
  r1.moveRight(sr.right());
  g1->setPos(r1.topLeft());
  r2 = g1->boundingRect();
  g1 = scene->addSimpleText(dashLine, normalFont);
  g1->setY(r1.bottom());
}