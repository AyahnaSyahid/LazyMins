// src/utils/posprinter.cpp
#include "posprinter.h"

#include <QPrinterInfo>
#include <QSerialPortInfo>
#include <QPageSize>
#include <QPageLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <QLocale>
#include <cmath>

// ==================== Singleton Implementation ====================

PosPrinter& PosPrinter::instance() {
    static PosPrinter instance;
    return instance;
}

PosPrinter::PosPrinter(QObject * p) : QObject(p), m_serialPort(this) {
  connect(&m_serialPort, &QSerialPort::errorOccurred, this, &PosPrinter::serialPortErrorHandler);
}

PosPrinter::~PosPrinter() {
    // Pastikan koneksi serial ditutup saat singleton dihancurkan
    if (m_serialPort.isOpen()) {
        m_serialPort.close();
    }
}

// ==================== Printer Discovery & Management ====================

QStringList PosPrinter::availablePrinters() const {
    QStringList printers;
    for (const auto& info : QPrinterInfo::availablePrinters()) {
        printers << info.printerName();
    }
    return printers;
}

bool PosPrinter::selectPrinter(const QString& printerName) {
    auto printers = availablePrinters();
    if (printers.contains(printerName)) {
        m_currentPrinter = printerName;
        m_lastError.clear();
        return true;
    }
    m_lastError = QString("Printer '%1' tidak ditemukan").arg(printerName);
    return false;
}

QString PosPrinter::currentPrinterName() const {
    return m_currentPrinter;
}

// ==================== Configuration Management ====================

bool PosPrinter::loadConfig(const QString& configPath) {
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = QString("Tidak bisa membuka file konfigurasi: %1").arg(configPath);
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject()) {
        m_lastError = "File konfigurasi tidak valid (bukan JSON object)";
        return false;
    }

    QJsonObject obj = doc.object();

    m_config.name             = obj.value("printer_name").toString("Thermal Printer");
    m_config.paperWidth       = obj.value("paper_width").toInt(76);
    m_config.characterWidth   = obj.value("character_width").toInt(12);
    m_config.marginLeft       = obj.value("margin_left").toDouble(3.0);
    m_config.marginRight      = obj.value("margin_right").toDouble(3.0);
    m_config.autoCut          = obj.value("auto_cut").toBool(true);
    m_config.fontFamily       = obj.value("font_family").toString("Courier");
    m_config.fontSize         = obj.value("font_size").toInt(9);
    m_config.supportsQR       = obj.value("supports_qr").toBool(false);
    m_config.supportsImage    = obj.value("supports_image").toBool(false);
    m_config.supportsBarcode  = obj.value("supports_barcode").toBool(false);
    m_config.supportsEscPos   = obj.value("supports_escpos").toBool(true);
    m_config.supportsPartialCut = obj.value("supports_partial_cut").toBool(true);
    m_config.supportsFullCut    = obj.value("supports_full_cut").toBool(false);

    return true;
}

void PosPrinter::saveConfig(const QString& configPath) const {
    QJsonObject obj;
    obj["printer_name"]        = m_config.name;
    obj["paper_width"]         = m_config.paperWidth;
    obj["character_width"]     = m_config.characterWidth;
    obj["margin_left"]         = m_config.marginLeft;
    obj["margin_right"]        = m_config.marginRight;
    obj["auto_cut"]            = m_config.autoCut;
    obj["font_family"]         = m_config.fontFamily;
    obj["font_size"]           = m_config.fontSize;
    obj["supports_qr"]         = m_config.supportsQR;
    obj["supports_image"]      = m_config.supportsImage;
    obj["supports_barcode"]    = m_config.supportsBarcode;
    obj["supports_escpos"]     = m_config.supportsEscPos;
    obj["supports_partial_cut"]= m_config.supportsPartialCut;
    obj["supports_full_cut"]   = m_config.supportsFullCut;

    QJsonDocument doc(obj);
    QFile file(configPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void PosPrinter::setPrinterConfig(const PrinterConfig& config) {
    m_config = config;
}

PrinterConfig PosPrinter::printerConfig() const {
    return m_config;
}

// ==================== ESC/POS Serial Connection ====================
QString PosPrinter::serialPortName() const {
  return m_serialPort.portName();
}

qint32 PosPrinter::serialBaudRate(QSerialPort::Directions directions) const {
  return m_serialPort.baudRate(directions);
}

bool PosPrinter::connectSerialPort(const QString& portName, int baudRate) {
    if(m_serialPort.isOpen()) {
      m_serialPort.close();
    }
    if (m_serialPort.portName() != portName ) m_serialPort.setPort(QSerialPortInfo(portName));
    if (m_serialPort.baudRate() != baudRate ) m_serialPort.setBaudRate(baudRate);
    m_serialPort.open(QIODevice::ReadWrite);
    return m_serialPort.isOpen();
}

bool PosPrinter::disconnectSerialPort() {
    if (m_serialPort.isOpen()) {
      m_serialPort.close();
    }
    return true;
}

bool PosPrinter::isSerialConnected() const {
    return m_serialPort.isOpen();
}

QStringList PosPrinter::availableSerialPorts() const {
    // Buat instance sementara hanya untuk list port
    QStringList pl;
    for(auto const& info : QSerialPortInfo::availablePorts()) {
      pl << info.portName();
    }
    return pl;
}

// ==================== ESC/POS Internal Helper ====================

bool PosPrinter::ensureEscPosReady() {
  return m_serialPort.isOpen();
}

// ==================== ESC/POS Commands ====================

bool PosPrinter::printReceiptViaEscPos(const Receipt& receipt) {
  if (!ensureEscPosReady()) return false;
  EscPosPrinter p(&m_serialPort);
  auto line = [](QChar ch, int size=40) { return QString(size, ch); }

  p << EscPosPrinter::init << EscPosPrinter::EncodingPC850
    << EscPosPrinter::PrintModes(EscPosPrinter::PrintModeDoubleWidth | EscPosPrinter::PrintModeDoubleHeight | EscPosPrinter::PrintModeEmphasized)
    << EscPosPrinter::JustificationCenter
    
  return true;
}

bool PosPrinter::testPrintViaEscPos() {
    if (!ensureEscPosReady()) return false;
    
    EscPosPrinter p(&m_serialPort);
    // Init(reset) the printer and set some encoding
    p << EscPosPrinter::init << EscPosPrinter::EncodingPC850;
    
    // Print some text with some formatting options, if a plain string "foo" is sent
    // it won't be handled by QCodec, it will send as raw data.
    p << EscPosPrinter::PrintModes(EscPosPrinter::PrintModeDoubleWidth | EscPosPrinter::PrintModeDoubleHeight | EscPosPrinter::PrintModeEmphasized)
      << EscPosPrinter::JustificationCenter
      << QStringLiteral("Some Text");

    p << "\n";

    // Printing QRCodes
    p << EscPosPrinter::JustificationCenter << EscPosPrinter::PrintModes(EscPosPrinter::PrintModeNone)
      << EscPosPrinter::QRCode(EscPosPrinter::QRCode::Model2, 5, EscPosPrinter::QRCode::M, "https://github.com/ceciletti/escpos-qt") << "\n"
      << EscPosPrinter::JustificationLeft;
    return true;
}



// ==================== ESC/POS Builder Sections ====================
/*****
void PosPrinter::buildEscPosHeader(EscPosBuilder& builder, const Receipt& receipt, int width) {
    EscPos::TextStyle namaTokoStyle { 
        .bold = true,
        .underline = false,
        .italic = false,
        .doubleWidth = true,
        .doubleHeight = true,
        .fontSize = EscPos::FontSize::Normal
          };

    builder.alignCenter();
    builder.horizontalLine('=', width);
    builder.textStyled("AKSARAJAYA", namaTokoStyle);
    builder.alignCenter()
      .text("Jl. Kapten Naseh No. 40, Tasikmalaya")
      .newline();
    builder.text("Telp : 0816171816").newline();
    builder.text("WA   : 0816171816").newline();
    builder.horizontalLine('=', width);

    // TODO: tambahkan alamat/telepon toko di sini jika perlu
    builder.alignLeft();
    builder.bold(true);
    builder.text("Nota No : " + receipt.invoiceNo).newline();
    builder.bold(false);
    builder.text("Tanggal : " + receipt.date + " " + receipt.time).newline();
    builder.text("Kasir   : ");
    builder.bold(true);
    builder.text(receipt.cashierName).newline();
    builder.bold(false);
    builder.horizontalLine('-', width);
}

void PosPrinter::buildEscPosCustomerInfo(EscPosBuilder& builder, const Receipt& receipt, int width) {
    if (receipt.customerName.isEmpty()) return;

    builder.horizontalLine('-', width);
    builder.text("PELANGGAN").newline();
    builder.horizontalLine('-', width);
    builder.text(receipt.customerName).newline();

    if (!receipt.customerPhone.isEmpty()) {
        builder.text(receipt.customerPhone).newline();
    }

    if (!receipt.customerAddress.isEmpty()) {
        // Potong jika terlalu panjang untuk lebar printer
        // TODO: tambahkan word-wrap jika alamat panjang
        builder.text(receipt.customerAddress.left(width)).newline();
    }
}

void PosPrinter::buildEscPosItems(EscPosBuilder& builder, const Receipt& receipt, int width) {
    builder.horizontalLine('-', width);
    builder.column("ITEM", "HARGA", width);
    builder.horizontalLine('-', width);

    for (const auto& item : receipt.items) {
        // Baris pertama: nama item (potong jika terlalu panjang)
        builder.text(item.description.left(width)).newline();

        // Baris kedua: qty x harga satuan = total
        QString qty = QString::number(item.quantity, 'f', 0)
                      + "x" + QString::number(item.unitPrice, 'f', 0);
        QString total = QString::number(item.totalPrice, 'f', 0);
        builder.column(qty, total, width);
    }

    // Biaya finishing jika ada
    if (!receipt.finishings.isEmpty()) {
        builder.newline();
        builder.text("Biaya Finishing:").newline();
        for (const auto& finishing : receipt.finishings) {
            builder.column("- " + finishing.name,
                           QString::number(finishing.cost, 'f', 0),
                           width);
        }
    }
}

void PosPrinter::buildEscPosTotals(EscPosBuilder& builder, const Receipt& receipt, int width) {
    builder.horizontalLine('-', width);
    builder.column("SUBTOTAL", QString::number(receipt.subtotal, 'f', 0), width);

    if (receipt.discount > 0) {
        builder.column("DISKON", "-" + QString::number(receipt.discount, 'f', 0), width);
    }

    if (receipt.tax > 0) {
        QString taxLabel = QString("PPN %1%").arg(receipt.taxRate * 100, 0, 'f', 0);
        builder.column(taxLabel, QString::number(receipt.tax, 'f', 0), width);
    }

    builder.horizontalLine('=', width);
    builder.bold(true);
    builder.column("TOTAL", QString::number(receipt.grandTotal, 'f', 0), width);
    builder.bold(false);
}

void PosPrinter::buildEscPosPaymentInfo(EscPosBuilder& builder, const Receipt& receipt, int width) {
    builder.horizontalLine('-', width);

    if (receipt.payments.size() > 1) {
        // Multi-payment: rincikan per termin beserta nama akun
        builder.text("RIWAYAT PEMBAYARAN:").newline();
        int idx = 1;
        for (const auto& pay : receipt.payments) {
            // Label: "T1 Kas Admin" — singkat agar muat di 40 char
            QString label = QString("  T%1 %2").arg(idx++).arg(pay.akunNama);
            builder.column(label, QString::number(pay.amount, 'f', 0), width);
        }
        builder.horizontalLine('-', width);
        builder.column("TOTAL BAYAR", QString::number(receipt.paidAmount, 'f', 0), width);

        if (receipt.remaining > 0) {
            builder.column("SISA TAGIHAN", QString::number(receipt.remaining, 'f', 0), width);
        }
    } else if (receipt.payments.size() == 1) {
        const auto& pay = receipt.payments.first();

        // Nama akun sebagai metode pembayaran (e.g. "Kas Admin", "Bank BRI")
        if (!pay.akunNama.isEmpty()) {
            builder.column("METODE", pay.akunNama, width);
        }

        if (pay.akunTipe == "cash") {
            // Tunai: tampilkan uang diterima dan kembalian jika ada
            if (pay.cashReceived > 0) {
                builder.column("TUNAI", QString::number(pay.cashReceived, 'f', 0), width);
            }
            if (pay.cashChange > 0) {
                builder.column("KEMBALI", QString::number(pay.cashChange, 'f', 0), width);
            }
        } else {
            // Transfer / ewallet / lainnya: cukup jumlah yang dibayar
            builder.column("DIBAYAR", QString::number(pay.amount, 'f', 0), width);
        }
    } else {
        // Belum ada payment (unpaid)
        builder.column("DIBAYAR", "0", width);
        if (receipt.remaining > 0) {
            builder.column("SISA TAGIHAN", QString::number(receipt.remaining, 'f', 0), width);
        }
    }

    QString statusLabel;
    if      (receipt.status == "paid")     statusLabel = "LUNAS";
    else if (receipt.status == "partial")  statusLabel = "CICILAN";
    else if (receipt.status == "unpaid")   statusLabel = "BELUM BAYAR";
    else if (receipt.status == "refunded") statusLabel = "REFUND";
    else                                   statusLabel = receipt.status.toUpper();

    builder.text("STATUS: " + statusLabel).newline();
}

void PosPrinter::buildEscPosFooter(EscPosBuilder& builder, const Receipt& receipt, int width) {
    builder.horizontalLine('=', width);
    builder.alignCenter();
    builder.text("Terima kasih atas pesanan Anda!").newline();

    if (!receipt.pickupDate.isEmpty()) {
        builder.text("Ambil: " + receipt.pickupDate).newline();
    }

    // TODO: tambahkan QR code di sini jika printer mendukung dan ada URL yang perlu ditampilkan
    // if (m_config.supportsQR) { builder.qrCode(...); }

    builder.text("Percetakan Maju Jaya (c) 2026").newline();
    builder.alignLeft();

    // Feed beberapa baris sebelum cut agar teks tidak terpotong
    builder.lineFeed(6);
}
  ***************/

// ==================== QPrinter Receipt Printing ====================

bool PosPrinter::printReceipt(const Receipt& receipt) {
    if (m_currentPrinter.isEmpty()) {
        m_lastError = "Tidak ada printer yang dipilih";
        return false;
    }

    QPrinter printer(QPrinterInfo::printerInfo(m_currentPrinter));
    printer.setPageSize(QPageSize(QSizeF(m_config.paperWidth, 200), QPageSize::Millimeter));
    auto layout = printer.pageLayout();
    layout.setMargins(QMarginsF(m_config.marginLeft, 0, m_config.marginRight, 0),
                      QPageLayout::OutOfBoundsPolicy::Clamp);
    printer.setPageLayout(layout);
    printer.setColorMode(QPrinter::GrayScale);

    QPainter painter;
    if (!painter.begin(&printer)) {
        m_lastError = "Gagal memulai proses printing";
        return false;
    }

    drawReceipt(painter, receipt, printer.pageLayout().pageSize());
    painter.end();
    m_lastError.clear();
    return true;
}

bool PosPrinter::printReceiptPreview(const Receipt& receipt) {
    QPrinter printer;
    printer.setOutputFormat(QPrinter::NativeFormat);
    printer.setPageSize(QPageSize(QSizeF(m_config.paperWidth, 200), QPageSize::Millimeter));

    QPainter painter;
    if (!painter.begin(&printer)) {
        m_lastError = "Gagal membuat preview";
        return false;
    }

    drawReceipt(painter, receipt, printer.pageLayout().pageSize());
    painter.end();
    return true;
}

bool PosPrinter::testPrint() {
    Receipt testReceipt;
    testReceipt.invoiceNo    = "TEST-001";
    testReceipt.date         = QDate::currentDate().toString("dd/MM/yyyy");
    testReceipt.time         = QTime::currentTime().toString("HH:mm");
    testReceipt.cashierName  = "Sistem";
    testReceipt.customerName = "Test Customer";
    testReceipt.status       = "paid";

    ReceiptItem item;
    item.description = "Test Product";
    item.quantity    = 1.0;
    item.unitPrice   = 100000.0;
    item.totalPrice  = 100000.0;
    testReceipt.items.append(item);

    testReceipt.subtotal   = 100000.0;
    testReceipt.tax        = 11000.0;
    testReceipt.grandTotal = 111000.0;

    // Simulasi single payment tunai
    ReceiptPayment pay;
    pay.paymentNumber = "PAY-TEST-001";
    pay.amount        = 111000.0;
    pay.cashReceived  = 120000.0;
    pay.cashChange    = 9000.0;
    pay.date          = QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm");
    pay.akunNama      = "Kas Admin";
    pay.akunTipe      = "cash";
    testReceipt.payments.append(pay);
    testReceipt.paidAmount = 111000.0;
    testReceipt.amountPaid = 111000.0; // kompatibilitas mundur
    testReceipt.remaining  = 0.0;
    testReceipt.change     = pay.cashChange;

    return printReceipt(testReceipt);
}

// ==================== Error Handling ====================

QString PosPrinter::lastError() const {
    return m_lastError;
}

void PosPrinter::clearError() {
    m_lastError.clear();
}

// ==================== QPainter Drawing ====================

void PosPrinter::drawReceipt(QPainter& painter, const Receipt& receipt, const QPageSize& pageSize) {
    int maxCharsPerLine = calculateMaxCharsPerLine();
    int y = 20; // Mulai dari Y=20 untuk margin atas

    drawHeader(painter, receipt, maxCharsPerLine, y);

    if (!receipt.customerName.isEmpty()) {
        drawCustomerInfo(painter, receipt, maxCharsPerLine, y);
    }

    drawItems(painter, receipt, maxCharsPerLine, y);
    drawTotals(painter, receipt, maxCharsPerLine, y);
    drawPaymentInfo(painter, receipt, maxCharsPerLine, y);
    drawFooter(painter, receipt, maxCharsPerLine, y);
}

void PosPrinter::drawHeader(QPainter& painter, const Receipt& receipt, int maxCharsPerLine, int& y) {
    QFont headerFont(m_config.fontFamily, m_config.fontSize + 2, QFont::Bold);
    painter.setFont(headerFont);
    painter.drawText(10, y, centerText("PERCETAKAN MAJU JAYA", maxCharsPerLine));

    y += 20;
    painter.setFont(QFont(m_config.fontFamily, m_config.fontSize - 2));
    painter.drawText(10, y, centerText("Invoice #" + receipt.invoiceNo, maxCharsPerLine));

    y += 15;
    painter.drawText(10, y, receipt.date + " " + receipt.time);

    y += 15;
    painter.drawText(10, y, "Kasir: " + receipt.cashierName);

    y += 15;
    drawLine(painter, "=", maxCharsPerLine, 10, y);
    y += 15;
}

void PosPrinter::drawCustomerInfo(QPainter& painter, const Receipt& receipt, int maxCharsPerLine, int& y) {
    QFont font(m_config.fontFamily, m_config.fontSize - 1);
    painter.setFont(font);

    drawLine(painter, "-", maxCharsPerLine, 10, y);
    y += 15;
    painter.drawText(10, y, "PELANGGAN");
    y += 15;
    drawLine(painter, "-", maxCharsPerLine, 10, y);
    y += 15;

    painter.drawText(10, y, receipt.customerName);
    y += 12;

    if (!receipt.customerPhone.isEmpty()) {
        painter.drawText(10, y, receipt.customerPhone);
        y += 12;
    }

    if (!receipt.customerAddress.isEmpty()) {
        // TODO: tambahkan word-wrap jika alamat panjang
        painter.drawText(10, y, receipt.customerAddress.left(maxCharsPerLine));
        y += 12;
    }

    y += 5; // Jarak sebelum section berikutnya
}

void PosPrinter::drawItems(QPainter& painter, const Receipt& receipt, int maxCharsPerLine, int& y) {
    QFont font(m_config.fontFamily, m_config.fontSize - 1);
    painter.setFont(font);

    drawLine(painter, "-", maxCharsPerLine, 10, y);
    y += 15;
    painter.drawText(10, y, leftAlignText("ITEM", "HARGA", maxCharsPerLine));
    y += 15;

    for (const auto& item : receipt.items) {
        painter.drawText(10, y, item.description.left(maxCharsPerLine - 10));
        y += 12;

        QString qty = QString::number(item.quantity, 'f', 0)
                      + "x" + QString::number(item.unitPrice, 'f', 0);
        QString total = QString::number(item.totalPrice, 'f', 0);
        painter.drawText(10, y, leftAlignText(qty, total, maxCharsPerLine));
        y += 12;
    }

    if (!receipt.finishings.isEmpty()) {
        y += 5;
        painter.drawText(10, y, "Biaya Finishing:");
        y += 12;

        for (const auto& finishing : receipt.finishings) {
            painter.drawText(10, y, leftAlignText("- " + finishing.name,
                                                  QString::number(finishing.cost, 'f', 0),
                                                  maxCharsPerLine));
            y += 12;
        }
    }

    y += 5;
}

void PosPrinter::drawTotals(QPainter& painter, const Receipt& receipt, int maxCharsPerLine, int& y) {
    QFont font(m_config.fontFamily, m_config.fontSize - 1);
    painter.setFont(font);

    drawLine(painter, "-", maxCharsPerLine, 10, y);
    y += 15;

    painter.drawText(10, y, leftAlignText("SUBTOTAL",
                                          QString::number(receipt.subtotal, 'f', 0),
                                          maxCharsPerLine));
    y += 15;

    if (receipt.discount > 0) {
        painter.drawText(10, y, leftAlignText("DISKON",
                                              "-" + QString::number(receipt.discount, 'f', 0),
                                              maxCharsPerLine));
        y += 15;
    }

    if (receipt.tax > 0) {
        QString taxLabel = QString("PPN %1%").arg(receipt.taxRate * 100, 0, 'f', 0);
        painter.drawText(10, y, leftAlignText(taxLabel,
                                              QString::number(receipt.tax, 'f', 0),
                                              maxCharsPerLine));
        y += 15;
    }

    drawLine(painter, "=", maxCharsPerLine, 10, y);
    y += 15;

    QFont boldFont(m_config.fontFamily, m_config.fontSize, QFont::Bold);
    painter.setFont(boldFont);
    painter.drawText(10, y, leftAlignText("TOTAL",
                                          QString::number(receipt.grandTotal, 'f', 0),
                                          maxCharsPerLine));
    y += 20;
}

void PosPrinter::drawPaymentInfo(QPainter& painter, const Receipt& receipt, int maxCharsPerLine, int& y) {
    QFont font(m_config.fontFamily, m_config.fontSize - 1);
    painter.setFont(font);

    drawLine(painter, "-", maxCharsPerLine, 10, y);
    y += 15;

    if (receipt.payments.size() > 1) {
        // Multi-payment: rincikan per termin beserta nama akun
        painter.drawText(10, y, "RIWAYAT PEMBAYARAN:");
        y += 12;

        int idx = 1;
        for (const auto& pay : receipt.payments) {
            QString label = QString("  T%1 %2").arg(idx++).arg(pay.akunNama);
            painter.drawText(10, y, leftAlignText(label,
                                                  QString::number(pay.amount, 'f', 0),
                                                  maxCharsPerLine));
            y += 12;
        }

        drawLine(painter, "-", maxCharsPerLine, 10, y);
        y += 12;

        painter.drawText(10, y, leftAlignText("TOTAL BAYAR",
                                              QString::number(receipt.paidAmount, 'f', 0),
                                              maxCharsPerLine));
        y += 15;

        if (receipt.remaining > 0) {
            painter.drawText(10, y, leftAlignText("SISA TAGIHAN",
                                                  QString::number(receipt.remaining, 'f', 0),
                                                  maxCharsPerLine));
            y += 15;
        }
    } else if (receipt.payments.size() == 1) {
        const auto& pay = receipt.payments.first();

        // Nama akun sebagai metode pembayaran
        if (!pay.akunNama.isEmpty()) {
            painter.drawText(10, y, leftAlignText("METODE", pay.akunNama, maxCharsPerLine));
            y += 15;
        }

        if (pay.akunTipe == "cash") {
            if (pay.cashReceived > 0) {
                painter.drawText(10, y, leftAlignText("TUNAI",
                                                      QString::number(pay.cashReceived, 'f', 0),
                                                      maxCharsPerLine));
                y += 15;
            }
            if (pay.cashChange > 0) {
                painter.drawText(10, y, leftAlignText("KEMBALI",
                                                      QString::number(pay.cashChange, 'f', 0),
                                                      maxCharsPerLine));
                y += 15;
            }
        } else {
            painter.drawText(10, y, leftAlignText("DIBAYAR",
                                                  QString::number(pay.amount, 'f', 0),
                                                  maxCharsPerLine));
            y += 15;
        }
    } else {
        painter.drawText(10, y, leftAlignText("DIBAYAR", "0", maxCharsPerLine));
        y += 15;

        if (receipt.remaining > 0) {
            painter.drawText(10, y, leftAlignText("SISA TAGIHAN",
                                                  QString::number(receipt.remaining, 'f', 0),
                                                  maxCharsPerLine));
            y += 15;
        }
    }

    QString statusLabel;
    if      (receipt.status == "paid")     statusLabel = "LUNAS";
    else if (receipt.status == "partial")  statusLabel = "CICILAN";
    else if (receipt.status == "unpaid")   statusLabel = "BELUM BAYAR";
    else if (receipt.status == "refunded") statusLabel = "REFUND";
    else                                   statusLabel = receipt.status.toUpper();

    painter.drawText(10, y, "STATUS: " + statusLabel);
    y += 20;
}

void PosPrinter::drawFooter(QPainter& painter, const Receipt& receipt, int maxCharsPerLine, int& y) {
    QFont font(m_config.fontFamily, m_config.fontSize - 2);
    painter.setFont(font);

    drawLine(painter, "=", maxCharsPerLine, 10, y);
    y += 15;

    painter.drawText(10, y, centerText("Terima kasih atas pesanan Anda!", maxCharsPerLine));
    y += 12;

    if (!receipt.pickupDate.isEmpty()) {
        painter.drawText(10, y, centerText("Ambil: " + receipt.pickupDate, maxCharsPerLine));
        y += 12;
    }

    painter.drawText(10, y, centerText("Percetakan Maju Jaya (c) 2026", maxCharsPerLine));
}

// ==================== Helper Methods ====================

QString PosPrinter::centerText(const QString& text, int width) const {
    int padding = (width - text.length()) / 2;
    if (padding < 0) padding = 0;
    return QString(padding, ' ') + text;
}

QString PosPrinter::leftAlignText(const QString& left, const QString& right, int width) const {
    int space = width - left.length() - right.length();
    if (space < 1) space = 1;
    return left + QString(space, ' ') + right;
}

// Perbaikan bug: Y sebelumnya selalu 0, sekarang diterima sebagai parameter
void PosPrinter::drawLine(QPainter& painter, const QString& ch, int width, int x, int y) const {
    painter.drawText(x, y, QString(width, ch[0]));
}

int PosPrinter::calculateMaxCharsPerLine() const {
    
    return m_config.maxCharsPerLine; // Hard Code 
    // return static_cast<int>(
        // (m_config.paperWidth - m_config.marginLeft - m_config.marginRight) /
        // (m_config.characterWidth / 10.0)
    // );
}

void PosPrinter::serialPortErrorHandler(QSerialPort::SerialPortError error) {
  qDebug() << "SerialPort Error";
}