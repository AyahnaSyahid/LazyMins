// src/utils/escpos.h
#pragma once

#include <QString>
#include <QByteArray>
#include <QList>
#include <QSerialPort>

// ==================== ESC/POS Constants ====================

namespace EscPos {
    // Control characters
    static constexpr const char ESC = '\x1B';
    static constexpr const char GS  = '\x1D';
    static constexpr const char CR  = '\x0D';
    static constexpr const char LF  = '\x0A';

    // Alignment
    enum class Alignment {
        Left   = 0x00,
        Center = 0x01,
        Right  = 0x02
    };

    // Font size (GS ! n)
    enum class FontSize {
        Normal     = 0x00,   // 1x1
        Double     = 0x11,   // 2x2
        Large      = 0x21,   // 2x3
        VeryLarge  = 0x31    // 3x3
    };

    // Text style
    struct TextStyle {
        bool bold         = false;
        bool underline    = false;
        bool italic       = false;
        bool doubleWidth  = false;
        bool doubleHeight = false;
        FontSize fontSize = FontSize::Normal;
    };

    // Barcode type
    enum class BarcodeType {
        UPC_A   = 65,
        UPC_E   = 66,
        EAN13   = 67,
        EAN8    = 68,
        CODE39  = 69,
        ITF     = 70,
        CODABAR = 71,
        CODE93  = 72,
        CODE128 = 73
    };

    // QR Code error correction
    enum class QRErrorCorrection {
        Low    = 48,   // ~7%
        Medium = 49,   // ~15%
        High   = 51    // ~30%
    };
    
    enum class CodePage {
        PC437          = 0,    // USA, Standard Europe (default pabrik)
        Katakana       = 1,
        PC850          = 2,    // Multilingual Latin-1
        PC860          = 3,    // Portuguese
        PC863          = 4,    // Canadian-French
        PC865          = 5,    // Nordic
        WPC1252        = 16,   // Windows-1252 (Latin I)
        PC866          = 17,   // Cyrillic
        PC852          = 18,   // Latin 2 (Central Europe)
        PC858          = 19,   // Multilingual + Euro symbol (sangat direkomendasikan)
        // Tambahan umum lainnya
        PC857          = 13,   // Turkish
        PC862          = 36,   // Hebrew
        PC864          = 37,   // Arabic
        WPC1250        = 45,   // Central Europe
        WPC1254        = 48,   // Turkish
        WPC1257        = 51    // Baltic
        // Anda dapat menambahkan lebih banyak sesuai kebutuhan printer
    };
}

// ==================== ESC/POS Command Builder ====================

class EscPosBuilder {
public:
    EscPosBuilder();

    // Initialization
    EscPosBuilder& reset();
    EscPosBuilder& initialize();

    // Text output
    EscPosBuilder& text(const QString& text);
    EscPosBuilder& textStyled(const QString& text, const EscPos::TextStyle& style);
    EscPosBuilder& newline(int count = 1);
    EscPosBuilder& lineFeed(int count = 1);

    // Alignment
    EscPosBuilder& align(EscPos::Alignment alignment);
    EscPosBuilder& alignLeft();
    EscPosBuilder& alignCenter();
    EscPosBuilder& alignRight();

    // Text style
    EscPosBuilder& bold(bool enable = true);
    EscPosBuilder& underline(bool enable = true);
    EscPosBuilder& italic(bool enable = true);
    EscPosBuilder& doubleHeight(bool enable = true);
    EscPosBuilder& doubleWidth(bool enable = true);
    EscPosBuilder& fontSize(EscPos::FontSize size);
    EscPosBuilder& resetStyle();

    // Lines & Separators
    EscPosBuilder& horizontalLine(char character = '-', int width = 32);
    EscPosBuilder& doubleLine(char character = '=', int width = 32);

    // Tables & Formatting
    EscPosBuilder& column(const QString& left, const QString& right, int width = 32);
    EscPosBuilder& tableRow(const QList<QString>& columns, const QList<int>& widths);

    // Barcode
    EscPosBuilder& barcode(const QString& data, EscPos::BarcodeType type,
                           int width = 3, int height = 50);

    // QR Code
    EscPosBuilder& qrCode(const QString& data, int moduleSize = 8,
                          EscPos::QRErrorCorrection errorCorrection = EscPos::QRErrorCorrection::High);

    // Image (raster graphics - GS v 0)
    EscPosBuilder& image(const QByteArray& imageData, int width, int height);

    // Drawer & Cutting
    EscPosBuilder& openDrawer(int pin = 0, int duration = 120);
    EscPosBuilder& partialCut();
    EscPosBuilder& fullCut();
    EscPosBuilder& cutAndFeed(int feedLines = 3);
    EscPosBuilder& codePage(EscPos::CodePage page);
    EscPosBuilder& setDefaultCodePage();

    // Buzzer
    EscPosBuilder& buzz(int duration = 100);

    // Paper status
    EscPosBuilder& queryPaperStatus();

    // Get final command
    QByteArray build() const;
    QString buildAsString() const;

    // Clear builder
    void clear();

private:
    QByteArray m_commands;
    EscPos::TextStyle m_currentStyle;
    EscPos::Alignment m_currentAlignment;
    uint8_t m_currentCharSize = 0x00;   // Combined GS ! value

    // Helper methods
    void applyCharSize();
    QString formatColumn(const QString& left, const QString& right, int width) const;
};

// ==================== ESC/POS Printer ====================

class EscPosPrinter {
public:
    // Constructor & Destructor
    explicit EscPosPrinter(const QString& portName = "");
    ~EscPosPrinter();

    // Connection management
    bool connect(const QString& portName, int baudRate = 19200);
    bool disconnect();
    bool isConnected() const;

    // Port management
    QStringList availablePorts() const;
    QString currentPort() const;
    qint32 currentBaud(int) const;

    // Send commands
    bool sendCommand(const QByteArray& command);
    bool sendCommand(const EscPosBuilder& builder);
    bool sendRawCommand(const QString& command);

    // Printer status
    bool checkStatus();
    QString lastError() const;
    void clearError();

    // Convenience methods
    bool printText(const QString& text);
    bool printLine(char character = '-', int width = 32);
    bool printSeparator();

    // Test print
    bool testPrint();

private:
    QSerialPort* m_port = nullptr;
    QString m_lastError;

    // Helper methods
    void setError(const QString& error);
};