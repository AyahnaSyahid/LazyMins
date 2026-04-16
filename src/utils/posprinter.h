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

struct ReceiptPayment {
    QString paymentNumber;      // PAY-20260320-00001
    double  amount       = 0.0;
    double  cashReceived = 0.0; // Hanya terisi jika akunTipe == "cash"
    double  cashChange   = 0.0; // Hanya terisi jika akunTipe == "cash"
    QString date;               // "dd/MM/yyyy HH:mm"
    QString notes;

    // Dari JOIN ke akun_transaksi
    QString akunNama;           // e.g. "Kas Admin", "Bank BRI"
    QString akunTipe;           // "cash" | "bank" | "ewallet" | "lainnya"
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
    QList<ReceiptItem>     items;
    QList<ReceiptFinishing> finishings;

    // Totals
    double subtotal   = 0.0;
    double discount   = 0.0;
    double taxRate    = 0.11;  // Default PPN 11%
    double tax        = 0.0;
    double grandTotal = 0.0;

    // Payment — satu invoice bisa memiliki banyak payment (cicilan/partial)
    QList<ReceiptPayment> payments;

    // Agregat dari tabel invoices (sudah dihitung di DB)
    double paidAmount = 0.0;    // Total yang sudah dibayar
    double remaining  = 0.0;    // Sisa tagihan (grandTotal - paidAmount)

    // Kembalian tunai dari payment terakhir yang cash (0 jika tidak ada)
    double change = 0.0;

    // Kolom lama — dipertahankan untuk kompatibilitas mundur dengan kode pemanggil
    // yang belum diupdate. Diisi sama dengan paidAmount.
    double amountPaid = 0.0;

    // Status
    QString status;      // "unpaid", "partial", "paid", "refunded"
    QString notes;
    QString pickupDate;
};

// ==================== Printer Configuration ====================

struct PrinterConfig {
    QString name;                   // Printer name
    int paperWidth = 76;           // Paper width in mm — TM-U220D default 76mm
    int characterWidth = 12;       // Character width in tenths of mm
    double marginLeft = 5.0;
    double marginRight = 5.0;
    bool autoCut = true;

    // Font settings
    QString fontFamily = "Courier";
    int fontSize = 10;

    // Feature flags — set sesuai kemampuan printer
    // TM-U220D: dot-matrix, tidak support QR, image, atau barcode modern
    bool supportsGrayscale = false;
    bool supportsEscPos = true;
    bool supportsQR = false;        // TM-U220D: tidak support
    bool supportsImage = false;     // TM-U220D: tidak support raster image
    bool supportsBarcode = false;   // TM-U220D: tidak support barcode modern
    bool supportsPartialCut = true; // TM-U220D: hanya partial cut
    bool supportsFullCut = false;   // TM-U220D: tidak support full cut
    int  maxCharsPerLine = 40;
};

// Preset config untuk EPSON TM-U220D
// Gunakan ini sebagai titik awal, lalu sesuaikan jika perlu
inline PrinterConfig defaultTMU220DConfig() {
    PrinterConfig cfg;
    cfg.name = "EPSON TM-U220D";
    cfg.paperWidth = 76;
    cfg.characterWidth = 12;
    cfg.marginLeft = 3.0;
    cfg.marginRight = 3.0;
    cfg.autoCut = true;
    cfg.fontFamily = "Courier";
    cfg.fontSize = 9;
    cfg.supportsGrayscale = false;
    cfg.supportsEscPos = true;
    cfg.supportsQR = false;
    cfg.supportsImage = false;
    cfg.supportsBarcode = false;
    cfg.supportsPartialCut = true;
    cfg.supportsFullCut = false;
    cfg.maxCharsPerLine = 40;
    return cfg;
}

// ==================== POS Printer Manager ====================

class PosPrinter {
public:
    // Singleton instance
    static PosPrinter& instance();

    // Printer Discovery & Management (QPrinter / system printer)
    QStringList availablePrinters() const;
    bool selectPrinter(const QString& printerName);
    QString currentPrinterName() const;

    // Printer Configuration
    bool loadConfig(const QString& configPath);
    void saveConfig(const QString& configPath) const;
    void setPrinterConfig(const PrinterConfig& config);
    PrinterConfig printerConfig() const;

    // ---- QPrinter (system/driver) methods ----
    bool printReceipt(const Receipt& receipt);
    bool printReceiptPreview(const Receipt& receipt);
    bool testPrint();

    // ---- ESC/POS Serial methods ----
    QString serialPortName() const;
    qint32 serialBaudRate(QSerialPort::Directions directions = QSerialPort::AllDirections) const;

    // Manajemen koneksi serial — lifecycle dikelola oleh PosPrinter
    bool connectSerialPort(const QString& portName, int baudRate = 9600);
    bool disconnectSerialPort();
    bool isSerialConnected() const;
    QStringList availableSerialPorts() const;

    // Kirim perintah ESC/POS langsung (untuk custom command dari luar)
    bool sendRawEscPosCommand(const EscPosBuilder& builder);

    // Cetak struk via serial ESC/POS
    // Format sama dengan printReceipt(), tapi output ke port serial
    // TODO: sesuaikan layout jika printer memiliki lebar berbeda
    bool printReceiptViaEscPos(const Receipt& receipt);

    // Test print via serial ESC/POS
    bool testPrintViaEscPos();

    // ---- Error Handling ----
    QString lastError() const;
    void clearError();

private:
    PosPrinter() = default;
    ~PosPrinter();
    PosPrinter(const PosPrinter&) = delete;
    PosPrinter& operator=(const PosPrinter&) = delete;

    // ESC/POS printer instance — lifecycle dikelola di sini
    EscPosPrinter* m_escPosPrinter = nullptr;

    // Internal helper: pastikan ESC/POS printer siap dipakai
    bool ensureEscPosReady();

    // Internal QPrinter drawing methods
    void drawReceipt(QPainter& painter, const Receipt& receipt, const QPageSize& pageSize);
    void drawHeader(QPainter& painter, const Receipt& receipt, int maxCharsPerLine, int& y);
    void drawCustomerInfo(QPainter& painter, const Receipt& receipt, int maxCharsPerLine, int& y);
    void drawItems(QPainter& painter, const Receipt& receipt, int maxCharsPerLine, int& y);
    void drawTotals(QPainter& painter, const Receipt& receipt, int maxCharsPerLine, int& y);
    void drawPaymentInfo(QPainter& painter, const Receipt& receipt, int maxCharsPerLine, int& y);
    void drawFooter(QPainter& painter, const Receipt& receipt, int maxCharsPerLine, int& y);

    // Internal ESC/POS builder methods
    // TODO: jika layout perlu diubah per-section, pisahkan ke method masing-masing
    void buildEscPosHeader(EscPosBuilder& builder, const Receipt& receipt, int width);
    void buildEscPosCustomerInfo(EscPosBuilder& builder, const Receipt& receipt, int width);
    void buildEscPosItems(EscPosBuilder& builder, const Receipt& receipt, int width);
    void buildEscPosTotals(EscPosBuilder& builder, const Receipt& receipt, int width);
    void buildEscPosPaymentInfo(EscPosBuilder& builder, const Receipt& receipt, int width);
    void buildEscPosFooter(EscPosBuilder& builder, const Receipt& receipt, int width);

    // Helper methods
    QString centerText(const QString& text, int width) const;
    QString leftAlignText(const QString& left, const QString& right, int width) const;
    void drawLine(QPainter& painter, const QString& ch, int width, int x, int y) const;
    int calculateMaxCharsPerLine() const;

    // Member variables
    QString m_currentPrinter;
    PrinterConfig m_config;
    QString m_lastError;
};
