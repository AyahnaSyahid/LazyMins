// src/utils/escpos.cpp
#include "escpos.h"

#include <QSerialPortInfo>
#include <QThread>
#include <QDebug>

// ==================== EscPosBuilder Implementation ====================

EscPosBuilder::EscPosBuilder()
    : m_currentAlignment(EscPos::Alignment::Left)
    , m_currentCharSize(0x00)
{
}

EscPosBuilder& EscPosBuilder::reset() {
    m_commands.clear();
    m_currentStyle = EscPos::TextStyle();
    m_currentAlignment = EscPos::Alignment::Left;
    m_currentCharSize = 0x00;
    return *this;
}

EscPosBuilder& EscPosBuilder::initialize() {
    m_commands.append(EscPos::ESC);
    m_commands.append('@');
    return *this;
}

EscPosBuilder& EscPosBuilder::text(const QString& text) {
    m_commands.append(text.toUtf8());
    return *this;
}

EscPosBuilder& EscPosBuilder::textStyled(const QString& text, const EscPos::TextStyle& style) {
    EscPos::TextStyle previousStyle = m_currentStyle;
    uint8_t previousCharSize = m_currentCharSize;

    m_currentStyle = style;

    // Apply all style differences
    if (style.bold != previousStyle.bold)          bold(style.bold);
    if (style.underline != previousStyle.underline) underline(style.underline);
    if (style.italic != previousStyle.italic)      italic(style.italic);
    if (style.doubleHeight != previousStyle.doubleHeight) doubleHeight(style.doubleHeight);
    if (style.doubleWidth != previousStyle.doubleWidth)   doubleWidth(style.doubleWidth);
    if (style.fontSize != previousStyle.fontSize)  fontSize(style.fontSize);

    this->text(text);

    // Restore previous style
    m_currentStyle = previousStyle;
    m_currentCharSize = previousCharSize;
    resetStyle();

    return *this;
}

EscPosBuilder& EscPosBuilder::newline(int count) {
    for (int i = 0; i < count; ++i) {
        m_commands.append(EscPos::CR);
        m_commands.append(EscPos::LF);
    }
    return *this;
}

EscPosBuilder& EscPosBuilder::lineFeed(int count) {
    for (int i = 0; i < count; ++i) {
        m_commands.append(EscPos::LF);
    }
    return *this;
}

EscPosBuilder& EscPosBuilder::align(EscPos::Alignment alignment) {
    m_currentAlignment = alignment;
    m_commands.append(EscPos::ESC);
    m_commands.append('a');
    m_commands.append(static_cast<char>(alignment));
    return *this;
}

EscPosBuilder& EscPosBuilder::alignLeft()   { return align(EscPos::Alignment::Left); }
EscPosBuilder& EscPosBuilder::alignCenter() { return align(EscPos::Alignment::Center); }
EscPosBuilder& EscPosBuilder::alignRight()  { return align(EscPos::Alignment::Right); }

EscPosBuilder& EscPosBuilder::bold(bool enable) {
    m_currentStyle.bold = enable;
    m_commands.append(EscPos::ESC);
    m_commands.append('E');
    m_commands.append(enable ? '\x01' : '\x00');
    return *this;
}

EscPosBuilder& EscPosBuilder::underline(bool enable) {
    m_currentStyle.underline = enable;
    m_commands.append(EscPos::ESC);
    m_commands.append('-');
    m_commands.append(enable ? '\x01' : '\x00');
    return *this;
}

EscPosBuilder& EscPosBuilder::italic(bool enable) {
    m_currentStyle.italic = enable;
    m_commands.append(EscPos::ESC);
    m_commands.append('4');
    m_commands.append(enable ? '\x01' : '\x00');
    return *this;
}

void EscPosBuilder::applyCharSize() {
    m_commands.append(EscPos::GS);
    m_commands.append('!');
    m_commands.append(static_cast<char>(m_currentCharSize));
}

EscPosBuilder& EscPosBuilder::doubleHeight(bool enable) {
    m_currentStyle.doubleHeight = enable;
    if (enable) m_currentCharSize |= 0x01;
    else        m_currentCharSize &= ~0x01;
    applyCharSize();
    return *this;
}

EscPosBuilder& EscPosBuilder::doubleWidth(bool enable) {
    m_currentStyle.doubleWidth = enable;
    if (enable) m_currentCharSize |= 0x10;
    else        m_currentCharSize &= ~0x10;
    applyCharSize();
    return *this;
}

EscPosBuilder& EscPosBuilder::fontSize(EscPos::FontSize size) {
    m_currentStyle.fontSize = size;
    m_currentCharSize = static_cast<uint8_t>(size);
    applyCharSize();
    return *this;
}

EscPosBuilder& EscPosBuilder::resetStyle() {
    m_currentStyle = EscPos::TextStyle();
    m_currentCharSize = 0x00;

    bold(false);
    underline(false);
    italic(false);
    applyCharSize();
    return *this;
}

EscPosBuilder& EscPosBuilder::horizontalLine(char character, int width) {
    QString line(width, character);
    text(line);
    newline();
    return *this;
}

EscPosBuilder& EscPosBuilder::doubleLine(char character, int width) {
    QString line(width, character);
    bold(true);
    text(line);
    bold(false);
    newline();
    return *this;
}

EscPosBuilder& EscPosBuilder::column(const QString& left, const QString& right, int width) {
    QString formatted = formatColumn(left, right, width);
    text(formatted);
    newline();
    return *this;
}

EscPosBuilder& EscPosBuilder::tableRow(const QList<QString>& columns, const QList<int>& widths) {
    if (columns.size() != widths.size()) return *this;

    for (int i = 0; i < columns.size(); ++i) {
        QString col = columns[i].left(widths[i]);
        col.append(QString(widths[i] - col.length(), ' '));
        text(col);
    }
    newline();
    return *this;
}

EscPosBuilder& EscPosBuilder::barcode(const QString& data, EscPos::BarcodeType type,
                                     int width, int height) {
    m_commands.append(EscPos::GS); m_commands.append('w'); m_commands.append(static_cast<char>(width));
    m_commands.append(EscPos::GS); m_commands.append('h'); m_commands.append(static_cast<char>(height));

    m_commands.append(EscPos::GS);
    m_commands.append('k');
    m_commands.append(static_cast<char>(type));
    m_commands.append(static_cast<char>(data.length()));
    m_commands.append(data.toUtf8());

    newline();
    return *this;
}

EscPosBuilder& EscPosBuilder::qrCode(const QString& data, int moduleSize,
                                    EscPos::QRErrorCorrection errorCorrection) {
    QByteArray qrData = data.toUtf8();
    int dataLength = qrData.length();

    // Set module size: GS ( k 3 0 1 C n
    m_commands.append(EscPos::GS); m_commands.append('('); m_commands.append('k');
    m_commands.append('\x03'); m_commands.append('\x00'); m_commands.append('1'); m_commands.append('C');
    m_commands.append(static_cast<char>(moduleSize));

    // Set error correction: GS ( k 3 0 1 E m
    m_commands.append(EscPos::GS); m_commands.append('('); m_commands.append('k');
    m_commands.append('\x03'); m_commands.append('\x00'); m_commands.append('1'); m_commands.append('E');
    m_commands.append(static_cast<char>(errorCorrection));

    // Store QR data: GS ( k pL pH 1 D data   (pL pH = dataLength + 2)
    int storeLen = dataLength + 2;
    m_commands.append(EscPos::GS); m_commands.append('('); m_commands.append('k');
    m_commands.append(static_cast<char>(storeLen & 0xFF));
    m_commands.append(static_cast<char>((storeLen >> 8) & 0xFF));
    m_commands.append('1'); m_commands.append('D');
    m_commands.append(qrData);

    // Print QR code: GS ( k 2 0 1 P 0
    m_commands.append(EscPos::GS); m_commands.append('('); m_commands.append('k');
    m_commands.append('\x02'); m_commands.append('\x00');
    m_commands.append('1'); m_commands.append('P'); m_commands.append('\x30');  // 48 = '0'

    newline();
    return *this;
}

EscPosBuilder& EscPosBuilder::image(const QByteArray& imageData, int width, int height) {
    // GS v 0 m xL xH yL yH data  (m=0 = normal raster)
    m_commands.append(EscPos::GS);
    m_commands.append('v');
    m_commands.append('0');
    m_commands.append('\x00');                                 // m = 0
    m_commands.append(static_cast<char>(width & 0xFF));
    m_commands.append(static_cast<char>((width >> 8) & 0xFF));
    m_commands.append(static_cast<char>(height & 0xFF));
    m_commands.append(static_cast<char>((height >> 8) & 0xFF));
    m_commands.append(imageData);

    newline();
    return *this;
}

EscPosBuilder& EscPosBuilder::openDrawer(int pin, int duration) {
    m_commands.append(EscPos::ESC);
    m_commands.append('p');
    m_commands.append(static_cast<char>(pin));
    m_commands.append(static_cast<char>(duration & 0xFF));
    m_commands.append(static_cast<char>((duration >> 8) & 0xFF));
    return *this;
}

EscPosBuilder& EscPosBuilder::partialCut() {
    m_commands.append(EscPos::GS);
    m_commands.append('V');
    m_commands.append('\x01');
    return *this;
}

EscPosBuilder& EscPosBuilder::fullCut() {
    m_commands.append(EscPos::GS);
    m_commands.append('V');
    m_commands.append('\x00');
    return *this;
}

EscPosBuilder& EscPosBuilder::cutAndFeed(int feedLines) {
    m_commands.append(EscPos::GS);
    m_commands.append('V');
    m_commands.append('\x41');          // 'A' = cut after feeding
    m_commands.append(static_cast<char>(feedLines));
    return *this;
}

EscPosBuilder& EscPosBuilder::buzz(int duration) {
    m_commands.append('\x07');
    return *this;
}

EscPosBuilder& EscPosBuilder::queryPaperStatus() {
    m_commands.append('\x10');
    m_commands.append('\x04');
    m_commands.append('\x04');
    return *this;
}

QByteArray EscPosBuilder::build() const {
    return m_commands;
}

QString EscPosBuilder::buildAsString() const {
    return QString::fromUtf8(m_commands);
}

void EscPosBuilder::clear() {
    m_commands.clear();
    m_currentStyle = EscPos::TextStyle();
    m_currentAlignment = EscPos::Alignment::Left;
    m_currentCharSize = 0x00;
}

QString EscPosBuilder::formatColumn(const QString& left, const QString& right, int width) const {
    int spaceBetween = width - left.length() - right.length();
    if (spaceBetween < 1) spaceBetween = 1;
    return left + QString(spaceBetween, ' ') + right;
}

// ==================== EscPosPrinter Implementation ====================

EscPosPrinter::EscPosPrinter(const QString& portName)
    : m_port(nullptr)
{
    if (!portName.isEmpty()) {
        connect(portName);
    }
}

EscPosPrinter::~EscPosPrinter() {
    disconnect();
}

bool EscPosPrinter::connect(const QString& portName, int baudRate) {
    if (m_port && m_port->isOpen()) {
        disconnect();
    }

    m_port = new QSerialPort();
    m_port->setPortName(portName);
    m_port->setBaudRate(baudRate);
    m_port->setDataBits(QSerialPort::Data8);
    m_port->setStopBits(QSerialPort::OneStop);
    m_port->setParity(QSerialPort::NoParity);
    m_port->setFlowControl(QSerialPort::NoFlowControl);

    if (!m_port->open(QIODevice::ReadWrite)) {
        setError(QString("Tidak bisa membuka port: %1").arg(portName));
        delete m_port;
        m_port = nullptr;
        return false;
    }

    m_lastError.clear();
    return true;
}

bool EscPosPrinter::disconnect() {
    if (m_port && m_port->isOpen()) {
        m_port->close();
        m_port->deleteLater();
        m_port = nullptr;
    }
    return true;
}

bool EscPosPrinter::isConnected() const {
    return m_port && m_port->isOpen();
}

QStringList EscPosPrinter::availablePorts() const {
    QStringList ports;
    for (const auto& info : QSerialPortInfo::availablePorts()) {
        ports << info.portName();
    }
    return ports;
}

QString EscPosPrinter::currentPort() const {
    return m_port && isConnected() ? m_port->portName() : "";
}

qint32 EscPosPrinter::currentBaud(int) const {
    return isConnected() ? m_port->baudRate() : -1;
}

bool EscPosPrinter::sendCommand(const QByteArray& command) {
    if (!isConnected()) {
        setError("Printer tidak terhubung");
        return false;
    }

    qint64 written = m_port->write(command);
    if (written != command.size()) {
        setError("Gagal mengirim perintah ke printer");
        return false;
    }

    if (!m_port->waitForBytesWritten(3000)) {
        setError("Timeout mengirim perintah ke printer");
        return false;
    }

    m_lastError.clear();
    return true;
}

bool EscPosPrinter::sendCommand(const EscPosBuilder& builder) {
    return sendCommand(builder.build());
}

bool EscPosPrinter::sendRawCommand(const QString& command) {
    return sendCommand(command.toUtf8());
}

bool EscPosPrinter::checkStatus() {
    if (!isConnected()) {
        setError("Printer tidak terhubung");
        return false;
    }

    EscPosBuilder builder;
    builder.queryPaperStatus();

    if (!sendCommand(builder)) return false;

    if (m_port->waitForReadyRead(1000)) {
        QByteArray response = m_port->readAll();
        qDebug() << "Printer status response:" << response.toHex();
        return true;
    }

    setError("Tidak ada response dari printer");
    return false;
}

QString EscPosPrinter::lastError() const {
    return m_lastError;
}

void EscPosPrinter::clearError() {
    m_lastError.clear();
}

bool EscPosPrinter::printText(const QString& text) {
    EscPosBuilder builder;
    builder.text(text).newline();
    return sendCommand(builder);
}

bool EscPosPrinter::printLine(char character, int width) {
    EscPosBuilder builder;
    builder.horizontalLine(character, width);
    return sendCommand(builder);
}

bool EscPosPrinter::printSeparator() {
    return printLine('=', 32);
}

bool EscPosPrinter::testPrint() {
    EscPosBuilder builder;

    builder.initialize();
    builder.lineFeed(1);

    // Header - Center alignment with large font
    builder.alignCenter();
    builder.fontSize(EscPos::FontSize::VeryLarge);
    builder.bold(true);
    builder.text("TEST PRINT").newline();
    builder.bold(false);
    builder.fontSize(EscPos::FontSize::Large);
    builder.text("ESC/POS Printer Test").newline(2);

    // Company / Info
    builder.fontSize(EscPos::FontSize::Normal);
    builder.text("Percetakan Maju Jaya").newline();
    builder.text("Tes Komprehensif Fitur Printer").newline();
    builder.horizontalLine('=', 48);
    builder.lineFeed(1);

    // Alignment Test
    builder.alignLeft();
    builder.text("1. Alignment Test:").newline();
    builder.alignLeft();   builder.text("Teks rata kiri (default)").newline();
    builder.alignCenter(); builder.text("Teks rata tengah").newline();
    builder.alignRight();  builder.text("Teks rata kanan").newline(2);

    // Text Styles Test
    builder.alignLeft();
    builder.text("2. Text Styles:").newline();

    EscPos::TextStyle style;

    style.bold = true;
    builder.textStyled("Bold Text", style).newline();

    style.bold = false; style.underline = true;
    builder.textStyled("Underlined Text", style).newline();

    style.underline = false; style.italic = true;
    builder.textStyled("Italic Text", style).newline();

    style.italic = false; style.doubleWidth = true;
    builder.textStyled("Double Width", style).newline();

    style.doubleWidth = false; style.doubleHeight = true;
    builder.textStyled("Double Height", style).newline(2);

    // Font Size Test
    builder.text("3. Font Size Variation:").newline();
    builder.fontSize(EscPos::FontSize::Normal);     builder.text("Normal Size").newline();
    builder.fontSize(EscPos::FontSize::Double);     builder.text("Double Size (2x2)").newline();
    builder.fontSize(EscPos::FontSize::Large);      builder.text("Large Size (2x3)").newline();
    builder.fontSize(EscPos::FontSize::VeryLarge);  builder.text("Very Large Size").newline(2);

    // Table / Column Test
    builder.text("4. Table & Column Formatting:").newline();
    builder.column("Item", "Harga", 32);
    builder.column("Nasi Goreng", "Rp 25.000", 32);
    builder.column("Es Teh Manis", "Rp 8.000", 32);
    builder.horizontalLine('-', 32);

    QList<QString> cols = {"Produk", "Qty", "Total"};
    QList<int>     wids = {18, 6, 8};
    builder.tableRow(cols, wids);
    builder.tableRow({"Pulpen", "2", "Rp 10.000"}, wids);
    builder.tableRow({"Buku Catatan", "1", "Rp 15.000"}, wids);
    builder.lineFeed(1);

    // Barcode Test
    builder.alignCenter();
    builder.text("5. Barcode:").newline();
    builder.barcode("1234567890128", EscPos::BarcodeType::EAN13, 3, 60);
    builder.lineFeed(1);
    builder.barcode("ABC123456", EscPos::BarcodeType::CODE128, 3, 50);
    builder.lineFeed(2);

    // QR Code Test
    builder.text("6. QR Code:").newline();
    builder.qrCode("https://example.com/test-print", 6, EscPos::QRErrorCorrection::High);
    builder.lineFeed(2);

    // Buzzer & Drawer (jika didukung)
    builder.text("7. Buzzer & Cash Drawer:").newline();
    builder.buzz(150);
    builder.openDrawer(0, 120);   // Buka laci kas (pin 0)
    builder.lineFeed(2);

    // Footer
    builder.alignCenter();
    builder.bold(true);
    builder.text("=== SEMUA FITUR TELAH DIUJI ===").newline();
    builder.bold(false);
    builder.text("Printer ESC/POS Berhasil!").newline(6);

    // Potong kertas
    builder.cutAndFeed(5);        // Potong setelah memberi jarak
    // builder.partialCut();      // Alternatif: potong parsial

    return sendCommand(builder);
}

void EscPosPrinter::setError(const QString& error) {
    m_lastError = error;
    qWarning() << "EscPosPrinter Error:" << error;
}