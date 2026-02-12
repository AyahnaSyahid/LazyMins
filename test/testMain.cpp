#include "src/widget/createinvoicedialog.h"
#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QLocale>

#include "src/invoiceprinter.h"
#include "src/databaseinterface.h"

int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  QLocale l(QLocale::English, QLocale::Indonesia);
  QLocale::setDefault(l);
  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "JUST-INV_DB");
  db.setDatabaseName(QString("%1/data/jinv.db").arg(app.applicationDirPath()));
  if(!db.open()) {
    qFatal() << "Unable to Open database" << db.databaseName();
  }
  QSqlQuery(db).exec("PRAGMA foreign_keys = ON;");
  
  auto &di = DatabaseInterface::instance();
  auto &ip = InvoicePrinter::instance();
  
  // ip.drawInvoice(di.getPrintInvoiceParams(12));
  
  auto d = CreateInvoiceDialog();
  d.open();
  return app.exec();
}