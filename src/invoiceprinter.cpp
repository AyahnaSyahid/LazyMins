#include "invoiceprinter.h"
#include "databaseinterface.h"
#include <QSerialPort>
#include <QFile>
#include <QApplication>
#include "escposprinter.h"


InvoicePrinter &InvoicePrinter::instance() {
  static InvoicePrinter invp;
  return invp;
}

using EscPosQt::EscPosPrinter;

void InvoicePrinter::drawInvoice(const PrintInvoiceParams& pip) const {
  QSerialPort port;
  port.setPortName("COM4");  // Sesuaikan dengan Device Manager (Windows) atau /dev/ttyUSBx (Linux)
  port.setBaudRate(QSerialPort::Baud9600);
  port.setDataBits(QSerialPort::Data8);
  port.setParity(QSerialPort::NoParity);
  port.setStopBits(QSerialPort::OneStop);
  port.setFlowControl(QSerialPort::HardwareControl);  // Direkomendasikan DTR/DSR untuk stabilitas
  
  QFile wtf(qApp->applicationDirPath() + "/wtf.bin");
  
  if(!wtf.open(QIODevice::ReadWrite)) {
    qDebug() << "Tidak dapat membuka koneksi ke printer";
    return ;
  }
  
  EscPosPrinter printer(&wtf);
  printer << EscPosPrinter::init
          << EscPosPrinter::EncodingPC850
          << EscPosPrinter::PrintModes(EscPosPrinter::PrintModeDoubleWidth | EscPosPrinter::PrintModeDoubleHeight | EscPosPrinter::PrintModeEmphasized)
          << EscPosPrinter::JustificationCenter
          << pip.storeInfo.storeName
          << "\n"
          << EscPosPrinter::PrintModes(EscPosPrinter::PrintModeNone)
          << pip.storeInfo.storeAddr
          << "\n"
          << pip.storeInfo.storePhone
          << "\n"
          << QString("=================================\n")
          << QString("Print Time: %1\n").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"))
          << QString("---------------------------------\n")
          << EscPosPrinter::JustificationLeft
          << QString("Tanggal : %1\n").arg(pip.invoiceDate)
          << QString("No      : %1\n").arg(pip.invoiceCode)
          << QString("Konsumen: %1\n").arg(pip.customerName)
          << QString("Admin   : %1\n").arg(pip.adminName)
          << QString("=================================\n")
          << QString("  Nama Barang           Harga    \n")
          << QString("---------------------------------\n");
  printer << EscPosPrinter::feed(8);
  
  port.waitForBytesWritten(1000);
  port.close();
}