#include "invoiceprinter.h"
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QSqlQuery>
#include <QSettings>
#include <QMessageBox>
#include <QLabel>
#include <QVBoxLayout>

InvoicePrinter::InvoicePrinter(QObject* par)
: QObject(par) {
}

InvoicePrinter::~InvoicePrinter() {}

void InvoicePrinter::printInvoice(int id) {}
void InvoicePrinter::openConfig() {}
void InvoicePrinter::openPreviewDialog(int id)
{
  QString invoiceCode;
  QSqlQuery q;
  q.prepare("SELECT * FROM invoices_summary WHERE invoice_id = ?");
  q.addBindValue(id);
  q.exec();
  if(!q.next()) {
    QMessageBox::information(nullptr, "Kesalahan", QString("Invoice dengan id %1\nTidak dapat ditemukan").arg(id));
    return;
  }
  
  QString invCode(q.value("invoice_code").toString()),
          cust(q.value("name").toString()),
          invDate(q.value("date").toString());
  int invOrdCount = q.value("order_count").toInt();
  
  
  QImage barcode = createBarcode(q.value("Kode").toString(), 945, 200);
  QImage prcLogo(":/images/sample-logo.png");
  bool pickHeight;
  double byw = 945.0 / prcLogo.width(),
         byh = 200.0 / prcLogo.height();
  pickHeight = byw > byh ? true : false;
  if(pickHeight) {
    prcLogo = prcLogo.scaledToHeight(200, Qt::SmoothTransformation);
  } else {
    prcLogo = prcLogo.scaledToWidth(945, Qt::SmoothTransformation);
  }
  
  QRect logoRect(barcode.rect().adjusted((945 - prcLogo.width()) / 2.0, 200, (945 - prcLogo.width()) / -2.0, 200)),
  
  QImage fullImg = QImage(945, 1200, QImage::Format_Grayscale8);
  
  fullImg.setDotsPerMeterX(11811);
  fullImg.setDotsPerMeterY(11811);
  
  QPainter painter(&fullImg);
  
  QFont normalFont("Calibri", 9, QFont::Medium),
        bolderFont("Calibri", 9, QFont::DemiBold);
  
  
  
  
  painter.fillRect(fullImg.rect(), Qt::white);
  painter.drawImage(QPoint(0, 0), barcode );
  painter.drawImage(barcode.rect().adjusted((945 - prcLogo.width()) / 2.0, 200, (945 - prcLogo.width()) / -2.0, 200), prcLogo);
  
  QDialog dg;
  QVBoxLayout dgLy;
  QLabel imgLabel(&dg);
  imgLabel.setPixmap(QPixmap::fromImage(fullImg.scaledToWidth(fullImg.width() / 300.0 * 96)));
  dgLy.addWidget(&imgLabel);
  dg.setLayout(&dgLy);
  dg.adjustSize();
  dg.exec();
}