#include "invoicemaker.h"

#include <QDateTime>
#include <QDialog>
#include <QFont>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLineF>
#include <QPainter>
#include <QPointF>
#include <QSizeF>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QGraphicsView>
#include <QtDebug>

#include "invoicegraphics.h"

QImage createBarcode(const QString &, int w, int h);

const QString SELECT_INVOICE_QUERY(R"--(
WITH PaymentSummary AS (
    SELECT invoice_id, 
           SUM(amount) AS total_paid, 
           COUNT(*) AS payment_count, 
           MAX(payment_date) AS last_payment_date
    FROM payments
    GROUP BY invoice_id
)
SELECT inv.invoice_id,
       COALESCE('INV-' || REPLACE(COALESCE(inv.invoice_date, ''), '-', '') || '-' || 
                CASE WHEN inv.custom_code IS NULL 
                     THEN printf('%03X', COALESCE(inv.daily_enum, 0)) 
                     ELSE SUBSTR('000', 1, 5 - LENGTH(COALESCE(inv.custom_code, ''))) || COALESCE(inv.custom_code, '') 
                END, 'INV-ERROR') AS invoice_code,
       inv.invoice_date,
       cust.name,
       cust.address,
       SUM(ord.width * ord.height * ord.quantity * ord.unit_price) - (SUM(ord.discount) + inv.discount) AS invoice_price,
       COALESCE(ps.total_paid, 0) AS paid,
       SUM(ord.width * ord.height * ord.quantity * ord.unit_price) - (SUM(ord.discount) + inv.discount) - COALESCE(ps.total_paid, 0) AS unpaid,
       COALESCE(ps.payment_count, 0) AS payment_count,
       SUM(ord.width * ord.height * ord.quantity * ord.unit_price) AS orders_price,
       SUM(ord.width * ord.height * ord.quantity * ord.production_cost) AS orders_cost,
       COUNT(DISTINCT ord.order_id) AS orders_count,
       SUM(ord.discount) AS items_discount,
       inv.discount AS invoice_discount,
       COALESCE(ps.last_payment_date, '1990-01-01') AS last_payment_date
FROM invoices inv
JOIN customers cust ON cust.customer_id = inv.customer_id
JOIN orders ord ON inv.invoice_id = ord.invoice_id
LEFT JOIN PaymentSummary ps ON ps.invoice_id = inv.invoice_id
WHERE inv.invoice_id = ?
GROUP BY inv.invoice_id;
   )--");

const QString SELECT_ORDERS_QUERY(R"--(SELECT ord.name AS name,
       ord.use_area AS use_area,
       pr.name AS product,
       ord.width,
       ord.height,
       ord.unit_price,
       ord.quantity,
       ord.discount,
       CASE WHEN ord.use_area THEN ord.width * ord.height * ord.unit_price * ord.quantity ELSE ord.unit_price * ord.quantity END as subt
  FROM orders ord
       JOIN
       products pr USING (
           product_id
       )
 WHERE invoice_id = ?)--");

// helper
void drawFooter(QPainter &painter, const QRectF &area,
                const InvoiceGraphics &invg);
void drawCompanySign(QPainter &painter, const QRectF &area,
                     const InvoiceGraphics &invg,
                     const QFont &baseFont = QFont("Roboto Mono", 8.0f));
void helper_drawLine(const QPointF &, const QPointF &, QPainter *, int);

QRectF getRect(QPainter &painter, const QString &str);

InvoiceMaker::InvoiceMaker(QObject *parent) : QObject(parent) {
  int fontId = QFontDatabase::addApplicationFont(":/fonts/printing/roboto.ttf");
}

void InvoiceMaker::showInvoice(int iid, QWidget *parentWidget) {
  QSqlQuery q;
  q.prepare(SELECT_INVOICE_QUERY);
  q.addBindValue(iid);
  q.exec();
  if (!q.next()) {
    return;
  }

  auto ic = q.value("invoice_code").toString();
  InvoiceGraphics ig(ic, "Admin");
  ig.ordersCount = q.value("orders_count").toInt();
  ig.totalItemsPrice = q.value("orders_price").toInt();
  ig.itemsDiscount = q.value("items_discount").toInt();
  ig.invoiceDiscount = q.value("invoice_discount").toInt();
  ig.paidValue = q.value("paid").toInt();
  ig.customerInfo[0] = q.value("name").toString();
  ig.customerInfo[1] = q.value("address").toString();

  // calcullate imageSize
  // QFontMetricsF fmet(QFont("Roboto Mono", 7));
  // qDebug() << "font height" << fmet.height();
  // ig.rOrderItem.setHeight(fmet.height() / 96 * 900);
  auto imgDeviceHeight = ig.rOrderItem.top();
  // qDebug() << ig.rOrderItem.height();
  imgDeviceHeight += ig.ordersCount * ig.rOrderItem.height();
  imgDeviceHeight += 450;  // footer height

  // drawing invoices
  QImage dev(945, imgDeviceHeight, QImage::Format_ARGB32);
  dev.fill(Qt::white);
  dev.setDotsPerMeterX(11811);
  dev.setDotsPerMeterY(11811);

  QPainter painter(&dev);
  painter.setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
  // load and drawCompany Logo
  QString logoPath(":/images/company-logo.svg");
  QImage companyLogo(logoPath);
  if (!companyLogo.isNull()) {
    QRectF tempRect = companyLogo
                          .scaled(ig.rLogo.width(), ig.rLogo.height() - 20,
                                  Qt::KeepAspectRatio, Qt::SmoothTransformation)
                          .rect();
    tempRect.moveCenter(ig.rLogo.center());
    painter.drawImage(tempRect, companyLogo);
  }

  // draw company info //
  drawCompanySign(painter, ig.rCompanySign, ig, QFont("Roboto Mono", 8.0f));

  // drawBarcode
  QImage barcodeImage = createBarcode(ig.invoiceCode, ig.rBarcodeSpace.width(),
                                      ig.rBarcodeSpace.height());
  if (!barcodeImage.isNull()) {
    painter.drawImage(ig.rBarcodeSpace, barcodeImage);
  }
  auto pen = painter.pen();
  auto font = QFont("Roboto Mono", 7);
  auto met = painter.fontMetrics();
  painter.setFont(font);
  auto bottom = ig.rBarcodeSpace.bottom() + 20.0f;
  QLineF hLine(43.0f, bottom, 902, bottom);
  helper_drawLine(hLine.p1(), hLine.p2(), &painter, 0);
  font.setPointSizeF(8.0f);
  painter.setFont(font);
  // ig.rCustomerInfo.adjust(43, 20,-43, 0
  painter.drawText(ig.rCustomerInfo, Qt::AlignLeft | Qt::AlignVCenter,
                   QString("%1\n%2").arg(QDateTime::currentDateTime().toString(
                                             "yyyy/MM/dd hh:mm"),
                                         ig.userName));
  painter.drawText(ig.rCustomerInfo, Qt::AlignRight | Qt::AlignVCenter,
                   QString("%1\n%2").arg(ig.customerInfo[0], ""));

  // drawLine
  hLine = QLineF(ig.rCustomerInfo.bottomLeft(), ig.rCustomerInfo.bottomRight());
  hLine.translate(0, -10.0f);
  helper_drawLine(hLine.p1(), hLine.p2(), &painter, 1);

  float lastY = hLine.y1(), distFromLine = 20.f;

  QString tabHeader[]{" NO CODE|FILE                           DISC    ",
                      "      UKURAN |   QTY   |   SATUAN   |   ", "SUBTOTAL"};

  // auto met = painter.fontMetrics();
  int fontHeight = met.height();
  int textWidth = met.horizontalAdvance(tabHeader[0]);

  QRectF tempRect2, tempRect;

  font.setPointSizeF(7.0);
  font.setWeight(40);
  painter.setFont(font);
  painter.setBrush(Qt::black);

  painter.drawText(QRectF(40.0f, lastY + distFromLine, textWidth, fontHeight),
                   Qt::AlignLeft, tabHeader[0], &tempRect);
  painter.drawText(tempRect.translated(0, fontHeight), Qt::AlignLeft,
                   tabHeader[1], &tempRect2);

  font.setBold(true);
  painter.setFont(font);
  met = painter.fontMetrics();

  painter.drawText(tempRect2.adjusted(tempRect2.width(), 0,
                                      met.horizontalAdvance(tabHeader[2]), 0),
                   tabHeader[2]);

  pen.setStyle(Qt::DashLine);
  pen.setWidth(2);
  painter.setPen(pen);

  hLine.translate(0, 110.0f);
  helper_drawLine(hLine.p1(), hLine.p2(), &painter, 3);

  // drawOrderItem
  pen.setStyle(Qt::SolidLine);
  painter.setPen(pen);

  painter.setBrush(Qt::black);
  font.setPointSizeF(7);
  font.setWeight(40.0f);
  painter.setFont(font);
  met = painter.fontMetrics();

  fontHeight = met.height();
  // qDebug() << "Font Height :" << met.boundingRect(QRect(),Qt::TextDontClip | Qt::TextWordWrap, "XXX\nXXXX\nXXX").height();
  tempRect = ig.rOrderItem;
  QRectF numRect = tempRect.adjusted(
      0, 0, met.horizontalAdvance("000") - tempRect.width(), -met.height() * 2);
  QRectF codeNameRect =
      numRect.adjusted(numRect.width() + met.averageCharWidth(), 0,
                       tempRect.width() - numRect.width(), 0);
  QRectF ukuranRect = codeNameRect.translated(0, met.height());
  ukuranRect.setWidth(met.horizontalAdvance("8.88x8.88|"));
  QRectF qtyRect = ukuranRect.translated(ukuranRect.width(), 0);
  qtyRect.setWidth(met.horizontalAdvance("  800.000|"));
  QRectF satuanRect = qtyRect.translated(qtyRect.width(), 0);
  satuanRect.setWidth(met.horizontalAdvance("  10.800.000|"));
  QRectF discRect = satuanRect.translated(satuanRect.width(), 0);
  QRectF subtotalRect = discRect.translated(0, met.height());
  subtotalRect.setRight(ig.rOrderItem.right());

  q.prepare(SELECT_ORDERS_QUERY);
  q.addBindValue(iid);
  q.exec();

  int nOrder = 1, orderItemOffsets = 114, order_quantity, order_unit_price,
      order_discount, order_subt;
  float order_width, order_height;
  QLocale loc;
  while (q.next()) {
    order_quantity = q.value("quantity").toInt();
    order_unit_price = q.value("unit_price").toInt();
    order_discount = q.value("discount").toInt();
    order_subt = q.value("subt").toInt();
    order_width = q.value("width").toDouble();
    order_height = q.value("height").toDouble();
    
    // painter.save();
    // painter.setBrush(Qt::red);
    // painter.drawRect(numRect);
    // painter.restore();
    painter.drawText(numRect, Qt::AlignRight,
                     QString("%1").arg(nOrder, 3, 10, QChar('0')));
    
    // painter.save();
    // painter.setBrush(Qt::green);
    // painter.drawRect(codeNameRect);
    // painter.restore();
    painter.drawText(codeNameRect, Qt::AlignLeft,
                     QString("%1|%2").arg(q.value("product").toString(),
                                          q.value("name").toString()));
    // painter.save();
    // painter.setBrush(Qt::blue);
    // painter.drawRect(ukuranRect);
    // painter.restore();
    if (q.value("use_area").toBool()) {
      painter.drawText(
          ukuranRect, Qt::AlignRight,
          QString("%1x%2|").arg(loc.toString(order_width, 'g', 3),
                                loc.toString(order_height, 'g', 3)));
    } else {
      painter.drawText(ukuranRect, Qt::AlignRight, "1|");
    }
    
    // painter.save();
    // painter.setBrush(Qt::gray);
    // painter.drawRect(qtyRect);
    // painter.restore();
    painter.drawText(qtyRect, Qt::AlignRight,
                     loc.toString(order_quantity) + "|");
    
    // painter.save();
    // painter.setBrush(Qt::yellow);
    // painter.drawRect(satuanRect);
    // painter.restore();
    painter.drawText(satuanRect, Qt::AlignRight,
                     loc.toString(order_unit_price) + "|");

    // painter.save();
    // painter.setBrush(Qt::red);
    // painter.drawRect(discRect);
    // painter.restore();
    painter.drawText(discRect, Qt::AlignRight, loc.toString(order_discount));
    
    
    // painter.save();
    // painter.setBrush(Qt::gray);
    // painter.drawRect(subtotalRect);
    // painter.restore();
    
    painter.save();
    font = painter.font();
    // font.setPointSizeF(8);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(subtotalRect, Qt::AlignRight, loc.toString(order_subt));
    painter.restore();

    nOrder++;

    numRect.translate(0, orderItemOffsets);
    ukuranRect.translate(0, orderItemOffsets);
    codeNameRect.translate(0, orderItemOffsets);
    qtyRect.translate(0, orderItemOffsets);
    satuanRect.translate(0, orderItemOffsets);
    discRect.translate(0, orderItemOffsets);
    subtotalRect.translate(0, orderItemOffsets);
  }

  hLine.translate(0, orderItemOffsets * ig.ordersCount + met.height());
  helper_drawLine(hLine.p1(), hLine.p2(), &painter, 4);

  QRectF footer(hLine.x1(), hLine.y1(), hLine.dx(), 460);
  footer.adjust(0, 10, 0, 0);
  
  drawFooter(painter, footer, ig);
  
  painter.end();

  QDialog *dg = new QDialog();
  QHBoxLayout *dgl = new QHBoxLayout;
  auto lview = new QGraphicsView(dg);
  auto lscene = new QGraphicsScene(lview);
  lview->setScene(lscene);
  // QLabel *lab = new QLabel(dg);
  // lab->setPixmap(QPixmap::fromImage(
      // dev.scaled(dev.width() * 96.0f / 200.0f, dev.height() * 96.0f / 200.0f, Qt::KeepAspectRatio,
                 // Qt::SmoothTransformation)));
  lscene->addPixmap(QPixmap::fromImage(
      dev.scaled(dev.width() * 96.0f / 200.0f, dev.height() * 96.0f / 200.0f, Qt::KeepAspectRatio,
                 Qt::SmoothTransformation)));
  dgl->addWidget(lview);
  // dgl->addWidget(lab);
  dg->setLayout(dgl);
  dg->setAttribute(Qt::WA_DeleteOnClose);
  dg->adjustSize();
  dg->exec();
}

/**
 * Calculates the bounding rectangle for a multi-line string using the painter's
 * font.
 * @param p The QPainter providing the font metrics.
 * @param s The string to measure, with lines separated by '\n'.
 * @return A QRectF with the maximum width of any line and total height of all
 * lines.
 */
QRectF getRect(QPainter &p, const QString &s) {
  if (s.isEmpty()) {
    return QRectF();
  }
  QStringList lines = s.split("\n");
  QFontMetricsF met = p.fontMetrics();
  qreal maxWidth = 0;
  for (const QString &line : lines) {
    maxWidth = qMax(maxWidth, met.horizontalAdvance(line));
  }
  qreal totalHeight = lines.size() * met.height();
  return QRectF(0, 0, maxWidth, totalHeight);
}

void drawFooter(QPainter &painter, const QRectF &area,
                const InvoiceGraphics &ig) {
  QString itemCountFormat("ITEMS : %1"), totalPriceFormat("TOTAL : %1"),
      totalOrderDiscountFormat("TOTAL DISC : %1"),
      invoiceDiscountFormat("INV DISC : %1"), paidFormat("TERBAYAR : %1"),
      unpaidFormat("SISA : %1");

  painter.save();
  QFont font("Roboto Mono", 7);
  font.setBold(true);
  QRectF temp;
  painter.setFont(font);
  QLocale loc;
  painter.drawText(area, Qt::AlignLeft | Qt::AlignTop,
                   itemCountFormat.arg(loc.toString(ig.ordersCount), 4));
  painter.drawText(area, Qt::AlignRight | Qt::AlignTop,
                   totalPriceFormat.arg(loc.toString(ig.totalItemsPrice), 13),
                   &temp);
  temp.translate(0, temp.height());
  temp.setWidth(area.width());
  temp.moveLeft(area.left());
  painter.drawText(
      temp, Qt::AlignRight | Qt::AlignTop,
      totalOrderDiscountFormat.arg(loc.toString(ig.itemsDiscount), 13));
  temp.translate(0, temp.height());
  painter.drawText(
      temp, Qt::AlignRight | Qt::AlignTop,
      invoiceDiscountFormat.arg(loc.toString(ig.invoiceDiscount), 13));
  temp.translate(0, temp.height());
  painter.drawText(temp, Qt::AlignRight | Qt::AlignTop,
                   paidFormat.arg(loc.toString(ig.paidValue), 13));
  auto rest = ig.totalItemsPrice - (ig.itemsDiscount + ig.invoiceDiscount) -
              ig.paidValue;
  temp.translate(0, temp.height());
  painter.drawText(temp, Qt::AlignRight | Qt::AlignTop,
                   unpaidFormat.arg(loc.toString(rest), 13));
  temp.translate(0, temp.height());
  painter.drawText(temp, Qt::AlignCenter,
                   "***************************************************");
  QLineF hLine(temp.topLeft(), temp.topRight());
  hLine.translate(0, temp.height() - 10);
  auto pen = painter.pen();
  pen.setWidth(3);
  painter.setPen(pen);
  painter.drawLine(hLine);
  temp.translate(0, 20);
  temp.setHeight(200);
  font.setBold(false);
  painter.setFont(font);
  QStringList cStatement;
  for(int i=0; i<4; ++i) {
    cStatement << InvoiceGraphics::clossingStatements[i];
  }
  painter.drawText(temp, Qt::AlignCenter | Qt::AlignTop | Qt::TextWordWrap, cStatement.join("\n"));
  helper_drawLine(temp.bottomLeft(), temp.bottomRight(), &painter, 4);
  painter.restore();
}

/**
 * Draws a company sign with a bold title and additional info within the
 * specified area. Scales the font if the text exceeds the area size.
 * @param painter The QPainter to use for drawing.
 * @param area The target rectangle for drawing.
 * @param invg InvoiceGraphics containing company info (expects at least 4
 * elements).
 * @param baseFont Base font for rendering (default: Roboto Mono, 8pt).
 */
void drawCompanySign(QPainter &painter, const QRectF &area,
                     const InvoiceGraphics &invg, const QFont &baseFont) {
  painter.save();

  // Prepare text
  QStringList restInfo = {invg.companyInfo[1], invg.companyInfo[2],
                          invg.companyInfo[3]};
  QString restText = restInfo.join("\n");
  if (restText.isEmpty()) {
    qWarning() << "Empty company info text";
    painter.restore();
    return;
  }

  // Set up fonts
  QFont titleFont = baseFont;
  titleFont.setBold(true);
  QFont infoFont = baseFont;
  infoFont.setBold(false);

  // Calculate initial text rectangles
  painter.setFont(titleFont);
  QRectF titleRect = getRect(painter, invg.companyInfo[0]);
  painter.setFont(infoFont);
  QRectF infoRect = getRect(painter, restText);

  // Combine rectangles for total size
  QRectF combinedRect(0, 0, qMax(titleRect.width(), infoRect.width()),
                      titleRect.height() + infoRect.height());

  // Scale if necessary
  float scaleFactor = 1.0f;
  QSizeF estSize = combinedRect.size();
  QSizeF requestedSize = area.size();
  if (estSize.width() > 0 && estSize.width() > requestedSize.width()) {
    QSizeF ns = estSize.scaled(requestedSize, Qt::KeepAspectRatio);
    scaleFactor = ns.width() / estSize.width();
    titleFont.setPointSizeF(titleFont.pointSizeF() * scaleFactor);
    infoFont.setPointSizeF(infoFont.pointSizeF() * scaleFactor);

    // Recalculate with scaled font
    painter.setFont(titleFont);
    titleRect = getRect(painter, invg.companyInfo[0]);
    painter.setFont(infoFont);
    infoRect = getRect(painter, restText);
    combinedRect = QRectF(0, 0, qMax(titleRect.width(), infoRect.width()),
                          titleRect.height() + infoRect.height());
  }

  // Center and draw text
  combinedRect.moveCenter(area.center());
  painter.setFont(titleFont);
  QRectF lastRect;
  painter.drawText(combinedRect, Qt::AlignLeft, invg.companyInfo[0], &lastRect);
  painter.setFont(infoFont);
  painter.drawText(combinedRect.adjusted(0, lastRect.height(), 0, 0),
                   Qt::AlignLeft, restText);

  painter.restore();
}

void helper_drawLine(const QPointF &start, const QPointF &end, QPainter *p,
                     int lineType) {
  p->save();
  auto pen = p->pen();
  auto hLine = QLineF(start, end);
  switch (lineType) {
    case 0:
      // big line solid & dash
      pen.setWidth(4);
      pen.setStyle(Qt::SolidLine);
      p->setPen(pen);
      p->drawLine(hLine);
      pen.setStyle(Qt::DashLine);
      p->setPen(pen);
      p->drawLine(hLine.translated(0.0f, 15.0f));
      break;
    case 1:
      // big line dash & solid
      pen.setWidth(4);
      pen.setStyle(Qt::DashLine);
      p->setPen(pen);
      p->drawLine(hLine);
      pen.setStyle(Qt::SolidLine);
      p->setPen(pen);
      p->drawLine(hLine.translated(0.0f, 15.0f));
      break;
    case 2:
      // single small dash
      pen.setWidth(2);
      pen.setStyle(Qt::DashLine);
      p->drawLine(hLine);
      break;
    case 3:
      // double small dash
      pen.setWidth(2);
      pen.setStyle(Qt::DashLine);
      p->drawLine(hLine);
      p->drawLine(hLine.translated(0, 5));
      break;
    case 4:
      // small dash and solid
      pen.setWidth(2);
      pen.setStyle(Qt::DashLine);
      p->drawLine(hLine);
      pen.setStyle(Qt::SolidLine);
      p->setPen(pen);
      p->drawLine(hLine.translated(0, 5));
      break;
  }
  p->restore();
}