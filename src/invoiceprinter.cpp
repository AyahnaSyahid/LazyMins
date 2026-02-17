#include "invoiceprinter.h"
#include "databaseinterface.h"
#include "widget/receiptpreviewdialog.h"
#include <QSerialPort>
#include <QFile>
#include <QApplication>
#include "escposprinter.h"

namespace { // unnamed local-linkage
  struct ItemLineParams { 
    QString name; 
    int qty;
    int unitPrice;
    int subTotal;
  };

  void printItem(EscPosQt::EscPosPrinter &dev, const ItemLineParams &p) {
    const int MAX_NAME_WIDTH = 32;
    
    // Pecah nama jika lebih dari 32 karakter
    QStringList nameLines;
    QString remainingName = p.name;
    
    while (remainingName.length() > MAX_NAME_WIDTH) {
        // Cari spasi terakhir sebelum posisi 32
        int breakPos = remainingName.left(MAX_NAME_WIDTH).lastIndexOf(' ');
        
        if (breakPos == -1) {
            // Tidak ada spasi, potong paksa di posisi 32
            breakPos = MAX_NAME_WIDTH;
        }
        
        nameLines.append(remainingName.left(breakPos).trimmed());
        remainingName = remainingName.mid(breakPos).trimmed();
    }
    
    // Tambahkan sisa nama
    if (!remainingName.isEmpty()) {
        nameLines.append(remainingName);
    }
    
    dev << EscPosQt::EscPosPrinter::JustificationLeft;
    
    // Print semua baris nama
    for (const QString &line : nameLines) {
        dev << line.toUtf8() << "\n";
    }
    
    // Format string qty dan subtotal
    QString qtyStr = QString("%L1x @%L2")
                        .arg(p.qty, 4)
                        .arg(p.unitPrice);
    QString subtotalStr = QString("Rp %L1\n")
                            .arg(p.subTotal);
    
    // Baris terpisah untuk qty dan subtotal
    // Set justifikasi left
    dev << EscPosQt::EscPosPrinter::JustificationLeft;
    
    // Print qty info
    dev << qtyStr.toUtf8();
    
    // Carriage return (kembali ke awal baris)
    dev << "\r";
    
    // Set justifikasi right
    dev << EscPosQt::EscPosPrinter::JustificationRight;
    
    // Print subtotal
    dev << subtotalStr.toUtf8();
    
    // Reset ke left align
    dev << EscPosQt::EscPosPrinter::JustificationLeft;
  }
};

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
  
  // QFile wtf(qApp->applicationDirPath() + "/wtf.bin");
  
  if(!port.open(QIODevice::ReadWrite)) {
    qDebug() << "Tidak dapat membuka koneksi ke printer";
    return ;
  }
  
  EscPosPrinter printer(&port);
  printer << EscPosPrinter::init
          << EscPosPrinter::EncodingPC850
          << EscPosPrinter::PrintModes(EscPosPrinter::PrintModeNone)
          << EscPosPrinter::JustificationCenter
          << QByteArray("=================================\n")
          << EscPosPrinter::PrintModes(EscPosPrinter::PrintModeDoubleWidth | EscPosPrinter::PrintModeDoubleHeight | EscPosPrinter::PrintModeEmphasized)
          << pip.storeInfo.storeName
          << "\n"
          << EscPosPrinter::PrintModes(EscPosPrinter::PrintModeNone)
          << pip.storeInfo.storeAddr
          << "\n"
          << pip.storeInfo.storePhone
          << "\n"
          << EscPosPrinter::JustificationCenter
          << QByteArray("=================================\n")
          << QString("tm : %1\n").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")).toUtf8()
          << QByteArray("---------------------------------\n")
          << EscPosPrinter::JustificationLeft
          << QString("Tanggal : %1\n").arg(pip.invoiceDate).toUtf8()
          << QString("No      : %1\n").arg(pip.invoiceCode).toUtf8()
          << QString("Konsumen: %1\n").arg(pip.customerName).toUtf8()
          << QString("        - %1\n").arg(pip.customerPhone.isEmpty() ? "  " : pip.customerPhone).toUtf8()
          << QString("Admin   : %1\n").arg(pip.adminName).toUtf8()
          << EscPosPrinter::JustificationCenter
          << QByteArray("=================================\n")
          << EscPosPrinter::JustificationLeft
          << QByteArray("Nama Barang\r")
          << EscPosPrinter::JustificationRight 
          << QByteArray("Harga     \n")
          << EscPosPrinter::JustificationCenter
          << QByteArray("---------------------------------\n");
  int total_nota = 0;
  for(const auto &item : pip.itemList) {
    ItemLineParams ilp { item.productName, item.unitQty, item.unitPrice, item.subTotal };
    printItem(printer, ilp);
    total_nota += item.subTotal;
  }
  printer << EscPosPrinter::JustificationCenter
          << QByteArray("---------------------------------\n")
          << EscPosPrinter::JustificationRight
          << EscPosPrinter::PrintModes(EscPosPrinter::PrintModeEmphasized)
          << QString("Total : Rp. %L1\n").arg(total_nota, 10).toUtf8()
          << EscPosPrinter::PrintModes(EscPosPrinter::PrintModeNone)
          << EscPosPrinter::JustificationCenter
          << QByteArray("---------------------------------\n")
          << QByteArray("Terimakasih atas kepercayaan anda\n")
          << QByteArray("Jangan Lupa Sholat 5 Waktu\n")
          << QByteArray("------------------\n")
          << EscPosPrinter::feed(8);
 
  port.waitForBytesWritten(1000);
  port.close();
}

QDialog *InvoicePrinter::receiptPreview(qlonglong invoice_id, QWidget *parent) const
{
  auto *rd = new ReceiptPreviewDialog(invoice_id, parent);
  return rd;
}