#include "receiptpreviewdialog.h"
#include "ui_receiptpreviewdialog.h"
#include "../invoicedatatype.h"
#include "../databaseinterface.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFont>
#include <QPen>
#include <QGraphicsLineItem>
#include <QLocale>
#include <QRectF>

namespace {
  QMap<QString, QFont> m_font;
  
  void initFont() {
    static bool initialized;
    if (!initialized) {
      m_font["normal"] = QFont("Consolas", 9);
      m_font["bold"] = QFont("Consolas", 9);
      m_font["bold"].setBold(true);
      m_font["big"] = QFont("Consolas", 14);
      m_font["big"].setBold(true);
      initialized = true;
    }
  }
  
  PrintInvoiceParams getParam(qlonglong inv) {
    return DatabaseInterface::instance().getPrintInvoiceParams(inv);
  }

  QRectF drawHeader(QGraphicsScene *scn, const StoreInfo &si) {
    qreal y;
    QRecF g1rect;
    auto &font = m_font;
    auto g1 = scn->addSimpleText("================================", font["normal"]);
    g1rect = g1->boundingRect();
    g1 = scn->addSimpleText(si.storeName, font("big"));
    auto r1 = g1.boundingRect();
    r1.moveCenter(g1rect.center());
    r1.top(g1rect.bottom());
    g1->setPos(r1.topLeft());
    g1 = scn->addSimpleText(si.storeAddr, font["normal"]);
    
    g1->setY(r1.bottom());
    r1 = g1.boundingRect();
    g1 = scn->addSimpleText(si.storePhone, font["normal"]);
    g1->setY(r1.bottom());
    r1 = g1.boundingRect();
    g1 = scn->addSimpleText("================================", font["normal"]);
    g1->setY(r1.bottom());
  }
}

ReceiptPreviewDialog::ReceiptPreviewDialog(qlonglong inv, QWidget *parent)
: ui(new Ui::ReceiptPreviewDialog), scene(new QGraphicsScene), m_ready(false), param(getParam(inv)), QDialog(parent)
{
  ui->setupUi(this);
  ui->rView->setScene(scene);
  auto &di = DatabaseInterface::instance();
  m_ready = !param.adminName.isEmpty();
  initFont();
  draw();
}

ReceiptPreviewDialog::~ReceiptPreviewDialog() { delete ui; }

void ReceiptPreviewDialog::draw()
{
  scene->clear();
  if (!m_ready) return ;
  boldFont.setBold(true);
  bigFont.setBold(true);
  QRectF sr(1,1,1,1);
  auto g1 = scene->addSimpleText("================================", normalFont);
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
  g1 = scene->addSimpleText("================================", normalFont);
  r1 = g1->boundingRect();
  r1.moveCenter(r2.center());
  r1.moveTop(r2.bottom());
  g1->setPos(r1.topLeft());
  g2 = scene->addSimpleText("Nama Barang", normalFont);
  r2 = g2->boundingRect();
  r2.setTopLeft(r1.bottomLeft());
  g2->setPos(r2.topLeft());
  g1 = scene->addSimpleText("Harga", normalFont);
  r1 = g1->boundingRect();
  r1.moveTop(r2.top());
  r1.moveRight(sr.right());
  g1->setPos(r1.topLeft());
  g1 = scene->addSimpleText("-----------------------------------", normalFont);
  g1->setPos(r2.bottomLeft());
  drawInvoiceItems()
}