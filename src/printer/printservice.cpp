#include "printservice.h"
#include "posprinter.h"
#include "src/dialogs/configureserialposdialog.h"
#include "src/managers/helpers.h"
#include <QSettings>
#include <QDebug>

PrintService::PrintService(QObject *parent) : QObject(parent)
{
    loadSettings();
}

PrintService& PrintService::instance() {
    static PrintService instance;
    return instance;
}

void PrintService::loadSettings()
{
    QSettings settings;
    // Disinkronkan dengan ConfigureSerialPosDialog.cpp (menggunakan Huruf Kapital)
    m_portName = settings.value("SerialPrinter/PortName").toString();
    m_baudRate = settings.value("SerialPrinter/BaudRate", 9600).toInt();
    m_serialPortDisabled = settings.value("printer/serialPortDisabled", false).toBool();
}

void PrintService::printToSerialRequested(const Receipt &rcp)
{
    if(m_serialPortDisabled) return;

    // 1. Masukkan ke antrean
    m_printQueue << rcp;

    // 2. Cek validitas konfigurasi menggunakan fungsi statis dari Dialog
    if(!ConfigureSerialPosDialog::hasValidConfig()) {
        auto dlg = new ConfigureSerialPosDialog();
        dlg->setAttribute(Qt::WA_DeleteOnClose); // Cegah Memory Leak
        
        // Koneksi saat dialog diterima (disimpan)
        connect(dlg, &QDialog::accepted, this, [this]() {
            this->loadSettings();
            this->printQueuedReceipts(); // Lanjutkan cetak antrean
        });
        
        dlg->show();
        return;
    }

    // 3. Jika konfigurasi sudah ada, pastikan koneksi printer siap
    auto &inst = PosPrinter::instance();
    if (inst.serialPortName() != m_portName) {
        if (!inst.connectSerialPort(m_portName, m_baudRate)) {
            emit unableToPrint("Gagal menghubungkan printer pada port: " + m_portName);
            return;
        }
    }

    // 4. Jalankan pencetakan
    printQueuedReceipts();
}

void PrintService::printQueuedReceipts()
{
    auto &inst = PosPrinter::instance();
    
    while(!m_printQueue.isEmpty()) {
        Receipt rcp = m_printQueue.takeFirst();
        
        if(inst.printReceiptViaEscPos(rcp)) {
            emit receiptPrinted(rcp);
        } else {
            // Jika gagal cetak di tengah jalan, masukkan kembali ke depan antrean (opsional)
            // m_printQueue.prepend(rcp);
            emit unableToPrint("Gagal mengirim data ke printer.");
            break;
        }
    }
}

void PrintService::printReceiptRequested(const Receipt &rcp)
{
    // Placeholder untuk logika printer sistem (Windows/Linux Spooler)
    // Untuk saat ini diarahkan ke serial jika diperlukan
    printToSerialRequested(rcp);
}

void PrintService::onPaymentCreated(int paymentId) {
    auto result = DBOperationHelper::paymentHasCompletePaidInvoice(paymentId);
    if (!result.ok) {
        emit unableToPrint(result.error);
        return ;
    }
    Receipt rcp;
    DBOperationHelper::loadInvoiceDataFast(result.data["invoice_id"].toInt(), &rcp);
    printToSerialRequested(rcp);
}

void PrintService::printInvoiceToSerial(int id) {
    Receipt rcp;
    DBOperationHelper::loadInvoiceDataFast(id, &rcp);
    printToSerialRequested(rcp);
}