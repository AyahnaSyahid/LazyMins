// src/utils/escpos.cpp
#include "escpos.h"

#include <QSerialPortInfo>
#include <QThread>
#include <QDebug>
#include <QImage>
#include <QBuffer>
#include <QByteArray>
#include <cmath>

// ==================== EscPosBuilder Implementation ====================

EscPosBuilder::EscPosBuilder() 
    : m_currentAlignment(EscPos::Alignment::Left) {
}

EscPosBuilder& EscPosBuilder::reset() {
    m_commands.clear();
    m_currentStyle = EscPos::TextStyle();
    m_currentAlignment = EscPos::Alignment::Left;
    return *this;
}

EscPosBuilder& EscPosBuilder::initialize() {
    // ESC @ - Initialize printer
    m_commands.append(EscPos::ESC);
    m_commands.append('@');
    return *this;
}

EscPosBuilder& EscPosBuilder::text(const QString& text) {
    m_commands.append(text.toUtf8());
    return *this;
}

EscPosBuilder& EscPosBuilder::textStyled(const QString& text, const EscPos::TextStyle& style) {
    // Save current style
    EscPos::TextStyle previousStyle = m_currentStyle;
    m_currentStyle = style;
    
    // Apply style
    if (style.bold != previousStyle.bold) {
        bold(style.bold);
    }
    if (style.underline != previousStyle.underline) {
        underline(style.underline);
    }
    if (style.doubleHeight != previousStyle.doubleHeight) {
        doubleHeight(style.doubleHeight);
    }
    if (style.doubleWidth != previousStyle.doubleWidth) {
        doubleWidth(style.doubleWidth);
    }
    
    // Add text
    this->text(text);
    
    // Restore previous style
    m_currentStyle = previousStyle;
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

EscPosBuilder& EscPosBuilder::alignLeft() {
    return align(EscPos::Alignment::Left);
}

EscPosBuilder& EscPosBuilder::alignCenter() {
    return align(EscPos::Alignment::Center);
}

EscPosBuilder& EscPosBuilder::alignRight() {
    return align(EscPos::Alignment::Right);
}

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

EscPosBuilder& EscPosBuilder::doubleHeight(bool enable) {
    m_currentStyle.doubleHeight = enable;
    m_commands.append(EscPos::GS);
    m_commands.append('!');
    m_commands.append(enable ? '\x10' : '\x00');
    return *this;
}

EscPosBuilder& EscPosBuilder::doubleWidth(bool enable) {
    m_currentStyle.doubleWidth = enable;
    m_commands.append(EscPos::GS);
    m_commands.append('!');
    m_commands.append(enable ? '\x20' : '\x00');
    return *this;
}

EscPosBuilder& EscPosBuilder::fontSize(EscPos::FontSize size) {
    m_currentStyle.fontSize = size;
    m_commands.append(EscPos::GS);
    m_commands.append('!');
    m_commands.append(static_cast<char>(size));
    return *this;
}

EscPosBuilder& EscPosBuilder::resetStyle() {
    m_commands.append(EscPos::ESC);
    m_commands.append('@');
    m_currentStyle = EscPos::TextStyle();
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
    if (columns.size() != widths.size()) {
        return *this;
    }
    
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
    // GS w - Set barcode width
    m_commands.append(EscPos::GS);
    m_commands.append('w');
    m_commands.append(static_cast<char>(width));
    
    // GS h - Set barcode height
    m_commands.append(EscPos::GS);
    m_commands.append('h');
    m_commands.append(static_cast<char>(height));
    
    // GS k - Print barcode
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
    // Function 1C - QR Code initialization
    m_commands.append(EscPos::GS);
    m_commands.append(')');
    m_commands.append('k');
    
    // Set module size
    m_commands.append(EscPos::GS);
    m_commands.append('(');
    m_commands.append('k');
    m_commands.append('\x03');
    m_commands.append('\x00');
    m_commands.append('1');
    m_commands.append('C');
    m_commands.append(static_cast<char>(moduleSize));
    
    // Set error correction level
    m_commands.append(EscPos::GS);
    m_commands.append('(');
    m_commands.append('k');
    m_commands.append('\x03');
    m_commands.append('\x00');
    m_commands.append('1');
    m_commands.append('E');
    m_commands.append(static_cast<char>(errorCorrection));
    
    // Store QR code data
    QByteArray qrData = data.toUtf8();
    int dataLength = qrData.length();
    
    m_commands.append(EscPos::GS);
    m_commands.append('(');
    m_commands.append('k');
    m_commands.append(static_cast<char>(dataLength & 0xFF));
    m_commands.append(static_cast<char>((dataLength >> 8) & 0xFF));
    m_commands.append('1');
    m_commands.append('D');
    m_commands.append('1');
    m_commands.append(qrData);
    
    // Print QR code
    m_commands.append(EscPos::GS);
    m_commands.append('(');
    m_commands.append('k');
    m_commands.append('\x02');
    m_commands.append('\x00');
    m_commands.append('1');
    m_commands.append('P');
    m_commands.append('0');
    
    newline();
    return *this;
}

EscPosBuilder& EscPosBuilder::image(const QByteArray& imageData, int width, int height) {
    // GS * - Print raster bit image
    m_commands.append(EscPos::GS);
    m_commands.append('*');
    m_commands.append(static_cast<char>(width & 0xFF));
    m_commands.append(static_cast<char>((width >> 8) & 0xFF));
    m_commands.append(static_cast<char>(height & 0xFF));
    m_commands.append(static_cast<char>((height >> 8) & 0xFF));
    m_commands.append(imageData);
    
    newline();
    return *this;
}

EscPosBuilder& EscPosBuilder::openDrawer(int pin, int duration) {
    // ESC p - Open drawer
    m_commands.append(EscPos::ESC);
    m_commands.append('p');
    m_commands.append(static_cast<char>(pin));
    m_commands.append(static_cast<char>(duration & 0xFF));
    m_commands.append(static_cast<char>((duration >> 8) & 0xFF));
    return *this;
}

EscPosBuilder& EscPosBuilder::partialCut() {
    // GS V - Partial cut
    m_commands.append(EscPos::GS);
    m_commands.append('V');
    m_commands.append('\x01');
    return *this;
}

EscPosBuilder& EscPosBuilder::fullCut() {
    // GS V - Full cut
    m_commands.append(EscPos::GS);
    m_commands.append('V');
    m_commands.append('\x00');
    return *this;
}

EscPosBuilder& EscPosBuilder::cutAndFeed(int feedLines) {
    // GS V A - Cut and feed
    m_commands.append(EscPos::GS);
    m_commands.append('V');
    m_commands.append('m');
    m_commands.append(static_cast<char>(feedLines));
    return *this;
}

EscPosBuilder& EscPosBuilder::buzz(int duration) {
    // BEL character for buzzer
    m_commands.append('\x07');
    return *this;
}

EscPosBuilder& EscPosBuilder::queryPaperStatus() {
    // DLE EOT n - Query paper status
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
}

QString EscPosBuilder::formatColumn(const QString& left, const QString& right, int width) const {
    int spaceBetween = width - left.length() - right.length();
    if (spaceBetween < 1) spaceBetween = 1;
    return left + QString(spaceBetween, ' ') + right;
}

// ==================== EscPosPrinter Implementation ====================

EscPosPrinter::EscPosPrinter(const QString& portName)
    : m_port(nullptr) {
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
    
    if (!sendCommand(builder)) {
        return false;
    }
    
    // Read response
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
    builder.alignCenter();
    builder.bold(true);
    builder.text("TEST PRINT").newline();
    builder.bold(false);
    builder.text("Percetakan Maju Jaya").newline();
    builder.horizontalLine('=', 32);
    builder.alignLeft();
    builder.text("Tanggal: Sekarang").newline();
    builder.text("Status: OK").newline();
    builder.horizontalLine('=', 32);
    builder.alignCenter();
    builder.text("Printer Berhasil Terhubung!").newline();
    builder.lineFeed(6);
    builder.partialCut();
    
    return sendCommand(builder);
}

void EscPosPrinter::setError(const QString& error) {
    m_lastError = error;
    qWarning() << "EscPosPrinter Error:" << error;
}