#include "src/widget/createinvoicedialog.h"
#include <QApplication>
#include <QLocale>

int main(int argc, char** argv)
{
  QLocale l(QLocale::English, QLocale::Indonesia);
  QLocale::setDefault(l);
  QApplication app(argc, argv);
  auto d = CreateInvoiceDialog();
  d.open();
  return app.exec();
}