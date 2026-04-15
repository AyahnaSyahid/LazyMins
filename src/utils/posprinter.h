// src/utils/posprinter.h
#pragma once

#include "escpos.h"

#include <QString>
#include <QList>
#include <QMap>
#include <QRect>
#include <QPrinter>
#include <QPainter>

// ==================== Receipt Structure ====================

struct ReceiptItem {
    QString description;
    double quantity = 1.0;
    double unitPrice = 0.0;
    double totalPrice = 0.0;
    QString unit = "pcs";
};

struct ReceiptFinishing {
    QString name;
    double cost = 0.0;
};

struct Receipt {
    // Header Info
    QString invoiceNo;
    QString date;
    QString time;
    QString cashierName;
    
    // Customer Info
    QString customerName;
    QString customerPhone;
    QString customerAddress;
    
    // Items
    QList<ReceiptItem> items;
    QList<ReceiptFinishing> finishings;
    
    // Payment
    double subtotal = 0.0;
    double discount = 0.0;
    double taxRate = 0.11;  // Default PPN 11%
    double tax = 0.0;
    double grandTotal = 0.0;
    
    // Payment Details
    double amountPaid = 0.0;
    double change = 0.0;
    QString paymentMethod;
    
    // Status
    QString status;  // "DRAFT", "PAID", "PARTIAL"
    QString notes;
    QString pickupDate;
};

// ==================== Printer Configuration ====================

struct PrinterConfig {
    QString name;                   // Printer name
    int paperWidth = 80;           // Paper width in mm (80mm = ~32 chars)
    int characterWidth = 12;       // Character width in mm
    double marginLeft = 5.0;
    double marginRight = 5.0;
    bool autoCut = true;
    int cutPosition = 0;           // Position from bottom
    
    // Font settings
    QString fontFamily = "Courier";
    int fontSize = 10;
    
    // Feature support
    bool supportsGrayscale = false;
    bool supportsEscPos = true;    // ESC/POS protocol
    bool supportsQR = false;
};

// ==================== POS Printer Manager ====================

class PosPrinter {
public:
    // Singleton instance
    static PosPrinter& instance();
    
    // Printer Discovery & Management
    QStringList availablePrinters() const;
    bool selectPrinter(const QString& printerName);
    QString currentPrinterName() const;
    
    // Printer Configuration
    bool loadConfig(const QString& configPath);
    void saveConfig(const QString& configPath) const;
    void setPrinterConfig(const PrinterConfig& config);
    PrinterConfig printerConfig() const;
    
    // Receipt Printing
    bool printReceipt(const Receipt& receipt);
    bool printReceiptPreview(const Receipt& receipt);
    
    // Test Print
    bool testPrint();
    
    // Error Handling
    QString lastError() const;
    void clearError();
    
     // ESC/POS specific methods
    bool sendRawEscPosCommand(const EscPosBuilder& builder);
    bool printReceiptViaEscPos(const Receipt& receipt);
    bool testPrintViaEscPos();
    
    // Serial port connection (for ESC/POS)
    bool connectSerialPort(const QString& portName, int baudRate = 19200);
    bool disconnectSerialPort();
    bool isSerialConnected() const;
    QStringList availableSerialPorts() const;
    
private:
    PosPrinter() = default;
    ~PosPrinter() = default;
    PosPrinter(const PosPrinter&) = delete;
    PosPrinter& operator=(const PosPrinter&) = delete;
    EscPosPrinter* m_escPosPrinter = nullptr;
    
    // Internal printing methods
    bool initializePrinter();
    void drawReceipt(QPainter& painter, const Receipt& receipt, const QPageSize& pageSize);
    void drawHeader(QPainter& painter, const Receipt& receipt, int maxCharsPerLine);
    void drawCustomerInfo(QPainter& painter, const Receipt& receipt, int maxCharsPerLine);
    void drawItems(QPainter& painter, const Receipt& receipt, int maxCharsPerLine);
    void drawTotals(QPainter& painter, const Receipt& receipt, int maxCharsPerLine);
    void drawPaymentInfo(QPainter& painter, const Receipt& receipt, int maxCharsPerLine);
    void drawFooter(QPainter& painter, const Receipt& receipt, int maxCharsPerLine);
    
    // Helper methods
    QString centerText(const QString& text, int width) const;
    QString leftAlignText(const QString& text, const QString& value, int width) const;
    void drawLine(QPainter& painter, const QString& char_ = "-", int width = 32) const;
    int calculateMaxCharsPerLine() const;
    
    // Member variables
    QString m_currentPrinter;
    PrinterConfig m_config;
    QString m_lastError;
};