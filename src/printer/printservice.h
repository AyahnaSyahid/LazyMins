#pragma once

#include "receipt.h"
#include <QObject>
#include <QList>

class PrintService : public QObject
{
    Q_OBJECT
public:
    static PrintService& instance();

public slots:
    void enableAutoPrint(bool enable);
    // Menangani permintaan cetak lewat id invoice
    void printInvoiceToSerial(int id);
    // Menangani permintaan cetak lewat id payment
    void onPaymentCreated(int id);
    // Menangani permintaan cetak umum
    void printReceiptRequested(const Receipt &rcp);
    // Menangani permintaan cetak khusus serial
    void printToSerialRequested(const Receipt &rcp);
    // Memuat ulang pengaturan dari QSettings
    void loadSettings();

signals:
    void unableToPrint(const QString& reason);
    void receiptPrinted(const Receipt &rcp);

private slots:
    // Memproses antrean cetak yang ada
    void printQueuedReceipts();

private:
    PrintService(QObject *parent = nullptr);
    PrintService(const PrintService&) = delete;
    PrintService& operator=(const PrintService&) = delete;

    QString m_portName;
    int     m_baudRate = -1;
    bool    m_serialPortDisabled = false;
    bool    m_setupRejected = false;
    bool    m_disableAutoPrint = false;
    QList<Receipt> m_printQueue;
};