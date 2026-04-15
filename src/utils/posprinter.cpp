// src/utils/posprinter.cpp
#include "posprinter.h"

#include <QPrinterInfo>
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
    
    // Load printer config
    m_config.name = obj.value("printer_name").toString("Thermal Printer");
    m_config.paperWidth = obj.value("paper_width").toInt(80);
    m_config.characterWidth = obj.value("character_width").toInt(12);
    m_config.marginLeft = obj.value("margin_left").toDouble(5.0);
    m_config.marginRight = obj.value("margin_right").toDouble(5.0);
    m_config.autoCut = obj.value("auto_cut").toBool(true);
    m_config.fontFamily = obj.value("font_family").toString("Courier");
    m_config.fontSize = obj.value("font_size").toInt(10);
    m_config.supportsQR = obj.value("supports_qr").toBool(false);
    m_config.supportsEscPos = obj.value("supports_escpos").toBool(true);
    
    return true;
}

void PosPrinter::saveConfig(const QString& configPath) const {
    QJsonObject obj;
    obj["printer_name"] = m_config.name;
    obj["paper_width"] = m_config.paperWidth;
    obj["character_width"] = m_config.characterWidth;
    obj["margin_left"] = m_config.marginLeft;
    obj["margin_right"] = m_config.marginRight;
    obj["auto_cut"] = m_config.autoCut;
    obj["font_family"] = m_config.fontFamily;
    obj["font_size"] = m_config.fontSize;
    obj["supports_qr"] = m_config.supportsQR;
    obj["supports_escpos"] = m_config.supportsEscPos;
    
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

// ==================== Receipt Printing ====================

bool PosPrinter::printReceipt(const Receipt& receipt) {
    if (m_currentPrinter.isEmpty()) {
        m_lastError = "Tidak ada printer yang dipilih";
        return false;
    }
    
    QPrinter printer(QPrinterInfo::printerInfo(m_currentPrinter));
    
    // Setup printer untuk thermal receipt
    printer.setPageSize(QPageSize(QSizeF(m_config.paperWidth, 200), QPageSize::Millimeter));
    auto _pageLayout = printer.pageLayout();
    _pageLayout.setMargins(QMarginsF(m_config.marginLeft, 0, m_config.marginRight, 0), QPageLayout::OutOfBoundsPolicy::Clamp);
    printer.setPageLayout(_pageLayout);
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
    // Untuk preview, gunakan QPrinter dalam mode PDF/preview
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
    testReceipt.invoiceNo = "TEST-001";
    testReceipt.date = QDate::currentDate().toString("dd/MM/yyyy");
    testReceipt.time = QTime::currentTime().toString("HH:mm");
    testReceipt.cashierName = "Sistem";
    testReceipt.customerName = "Test Customer";
    testReceipt.status = "TEST";
    
    ReceiptItem item;
    item.description = "Test Product";
    item.quantity = 1.0;
    item.unitPrice = 100000.0;
    item.totalPrice = 100000.0;
    testReceipt.items.append(item);
    
    testReceipt.subtotal = 100000.0;
    testReceipt.tax = 11000.0;
    testReceipt.grandTotal = 111000.0;
    
    return printReceipt(testReceipt);
}

// ==================== Error Handling ====================

QString PosPrinter::lastError() const {
    return m_lastError;
}

void PosPrinter::clearError() {
    m_lastError.clear();
}

// ==================== Internal Drawing Methods ====================

void PosPrinter::drawReceipt(QPainter& painter, const Receipt& receipt, const QPageSize& pageSize) {
    int maxCharsPerLine = calculateMaxCharsPerLine();
    int y = 10;
    
    // Draw sections
    drawHeader(painter, receipt, maxCharsPerLine);
    y += 40;
    
    if (!receipt.customerName.isEmpty()) {
        drawCustomerInfo(painter, receipt, maxCharsPerLine);
        y += 50;
    }
    
    drawItems(painter, receipt, maxCharsPerLine);
    y += 30;
    
    drawTotals(painter, receipt, maxCharsPerLine);
    y += 50;
    
    drawPaymentInfo(painter, receipt, maxCharsPerLine);
    y += 40;
    
    drawFooter(painter, receipt, maxCharsPerLine);
}

void PosPrinter::drawHeader(QPainter& painter, const Receipt& receipt, int maxCharsPerLine) {
    QFont headerFont(m_config.fontFamily, m_config.fontSize + 2, QFont::Bold);
    painter.setFont(headerFont);
    
    int y = 20;
    painter.drawText(10, y, centerText("PERCETAKAN MAJU JAYA", maxCharsPerLine));
    
    y += 20;
    painter.setFont(QFont(m_config.fontFamily, m_config.fontSize - 2));
    painter.drawText(10, y, centerText("Invoice #" + receipt.invoiceNo, maxCharsPerLine));
    
    y += 15;
    painter.drawText(10, y, receipt.date + " " + receipt.time);
    
    y += 15;
    painter.drawText(10, y, "Kasir: " + receipt.cashierName);
    
    y += 15;
    drawLine(painter, "=", maxCharsPerLine);
}

void PosPrinter::drawCustomerInfo(QPainter& painter, const Receipt& receipt, int maxCharsPerLine) {
    QFont font(m_config.fontFamily, m_config.fontSize - 1);
    painter.setFont(font);
    
    int y = 120;
    
    drawLine(painter, "-", maxCharsPerLine);
    y += 15;
    
    painter.drawText(10, y, "PELANGGAN");
    y += 15;
    
    drawLine(painter, "-", maxCharsPerLine);
    y += 15;
    
    painter.drawText(10, y, receipt.customerName);
    y += 12;
    
    if (!receipt.customerPhone.isEmpty()) {
        painter.drawText(10, y, receipt.customerPhone);
        y += 12;
    }
    
    if (!receipt.customerAddress.isEmpty()) {
        // Wrap text if needed
        painter.drawText(10, y, receipt.customerAddress.left(maxCharsPerLine));
    }
}

void PosPrinter::drawItems(QPainter& painter, const Receipt& receipt, int maxCharsPerLine) {
    QFont font(m_config.fontFamily, m_config.fontSize - 1);
    painter.setFont(font);
    
    int y = 190;
    
    drawLine(painter, "-", maxCharsPerLine);
    y += 15;
    
    // Header
    painter.drawText(10, y, leftAlignText("ITEM", "HARGA", maxCharsPerLine));
    y += 15;
    
    // Items
    for (const auto& item : receipt.items) {
        QString desc = item.description.left(maxCharsPerLine - 10);
        painter.drawText(10, y, desc);
        y += 12;
        
        QString qty = QString::number(item.quantity, 'f', 0) + "x" + QString::number(item.unitPrice, 'f', 0);
        QString total = QString::number(item.totalPrice, 'f', 0);
        painter.drawText(10, y, leftAlignText(qty, total, maxCharsPerLine));
        y += 12;
    }
    
    // Finishing charges
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
}

void PosPrinter::drawTotals(QPainter& painter, const Receipt& receipt, int maxCharsPerLine) {
    QFont font(m_config.fontFamily, m_config.fontSize - 1);
    painter.setFont(font);
    
    int y = 380;
    
    drawLine(painter, "-", maxCharsPerLine);
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
    
    drawLine(painter, "=", maxCharsPerLine);
    y += 15;
    
    QFont boldFont(m_config.fontFamily, m_config.fontSize, QFont::Bold);
    painter.setFont(boldFont);
    painter.drawText(10, y, leftAlignText("TOTAL", 
                                          QString::number(receipt.grandTotal, 'f', 0), 
                                          maxCharsPerLine));
}

void PosPrinter::drawPaymentInfo(QPainter& painter, const Receipt& receipt, int maxCharsPerLine) {
    QFont font(m_config.fontFamily, m_config.fontSize - 1);
    painter.setFont(font);
    
    int y = 490;
    
    drawLine(painter, "-", maxCharsPerLine);
    y += 15;
    
    painter.drawText(10, y, leftAlignText("TUNAI", 
                                          QString::number(receipt.amountPaid, 'f', 0), 
                                          maxCharsPerLine));
    y += 15;
    
    if (receipt.change > 0) {
        painter.drawText(10, y, leftAlignText("KEMBALI", 
                                              QString::number(receipt.change, 'f', 0), 
                                              maxCharsPerLine));
        y += 15;
    }
    
    // Status
    painter.drawText(10, y, "STATUS: " + receipt.status);
}

void PosPrinter::drawFooter(QPainter& painter, const Receipt& receipt, int maxCharsPerLine) {
    QFont font(m_config.fontFamily, m_config.fontSize - 2);
    painter.setFont(font);
    
    int y = 580;
    
    drawLine(painter, "=", maxCharsPerLine);
    y += 15;
    
    painter.drawText(10, y, centerText("Terima kasih atas pesanan Anda!", maxCharsPerLine));
    y += 12;
    
    if (!receipt.pickupDate.isEmpty()) {
        painter.drawText(10, y, centerText("Ambil: " + receipt.pickupDate, maxCharsPerLine));
        y += 12;
    }
    
    painter.drawText(10, y, centerText("Percetakan Maju Jaya © 2026", maxCharsPerLine));
}

// ==================== Helper Methods ====================

QString PosPrinter::centerText(const QString& text, int width) const {
    int padding = (width - text.length()) / 2;
    if (padding < 0) padding = 0;
    return QString(padding, ' ') + text;
}

QString PosPrinter::leftAlignText(const QString& text, const QString& value, int width) const {
    int spaceBetween = width - text.length() - value.length();
    if (spaceBetween < 1) spaceBetween = 1;
    return text + QString(spaceBetween, ' ') + value;
}

void PosPrinter::drawLine(QPainter& painter, const QString& char_, int width) const {
    painter.drawText(10, 0, QString(width, char_[0]));
}

int PosPrinter::calculateMaxCharsPerLine() const {
    return (m_config.paperWidth - m_config.marginLeft - m_config.marginRight) / 
           (m_config.characterWidth / 10.0);
}