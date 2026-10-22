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
    m_disableAutoPrint = settings.value("printer/disableAutoPrint", false).toBool();
}

void PrintService::enableAutoPrint(bool enable)
{
    QSettings settings;
    settings.setValue("printer/disableAutoPrint", !enable);
    m_disableAutoPrint = !enable;
    settings.sync();
}


void PrintService::printToSerialRequested(const Receipt &rcp)
{
    if(m_serialPortDisabled) {
        emit unableToPrint("Printer Serial di nonaktifkan.");
        return ;
    }
    // 1. Masukkan ke antrean
    m_printQueue << rcp;

    // 2. Cek validitas konfigurasi menggunakan fungsi statis dari Dialog
    if(!ConfigureSerialPosDialog::hasValidConfig()) {
        if (m_setupRejected) return;
        auto dlg = new ConfigureSerialPosDialog();
        dlg->setAttribute(Qt::WA_DeleteOnClose); // Cegah Memory Leak
        
        // Koneksi saat dialog diterima (disimpan)
        connect(dlg, &QDialog::accepted, this, [this]() {
            this->loadSettings();
            this->printQueuedReceipts(); // Lanjutkan cetak antrean
        });

        // Catat penolakan konfig pada sesi ini
        connect(dlg, &QDialog::rejected, this, [this]() {
            this->m_setupRejected = true;
        });
        
        dlg->exec();
        return;
    }

    // 3. Jika konfigurasi sudah ada, pastikan koneksi printer siap
    auto &inst = PosPrinter::instance();
    if (inst.serialPortName() != m_portName) {
        if (!inst.connectSerialPort(m_portName, m_baudRate)) {
            auto cat = inst.errorCategory();
            if (cat == PrinterError::PortNotFound || cat == PrinterError::DeviceDisconnected) {
                emit unableToPrint(
                    "Printer tidak terhubung atau dicabut dari port USB.\n\n"
                    "Pastikan printer sudah dicolokkan dan menyala.\n\n"
                    "Port yang dikonfigurasi: " + m_portName + "\n" + inst.errorCategoryToString(cat) + "\n\n"
                    "Buka Pengaturan → Printer untuk mengkonfigurasi ulang."
                );
            } else {
                emit unableToPrint("Gagal menghubungkan printer pada port: " + m_portName + "\n\n" + inst.errorCategoryToString(cat));
            }
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
            // If the printer was disconnected, try to reconnect and retry once
            auto cat = inst.errorCategory();
            QString errMsg = inst.lastError();
            
            if (cat == PrinterError::DeviceDisconnected ||
                cat == PrinterError::NotConnected ||
                cat == PrinterError::WriteTimeout ||
                cat == PrinterError::WriteFailed) {
                
                qWarning() << "Print failed with category" << static_cast<int>(cat)
                           << "— attempting reconnect to" << m_portName;
                
                // Reconnect and retry once
                inst.clearError();
                if (inst.connectSerialPort(m_portName, m_baudRate)) {
                    if (inst.printReceiptViaEscPos(rcp)) {
                        emit receiptPrinted(rcp);
                        continue;  // success on retry — proceed to next
                    }
                    // Retry also failed — fall through to error
                    errMsg = inst.lastError();
                    cat = inst.errorCategory();
                }
            }
            
            // Build a context-aware error message based on the category
            QString detail = errMsg.isEmpty() ? inst.errorCategoryToString(cat) : errMsg;
            
            if (cat == PrinterError::PortNotFound || cat == PrinterError::DeviceDisconnected) {
                emit unableToPrint(
                    "Printer tidak terhubung atau dicabut dari port USB.\n\n"
                    "Pastikan printer sudah dicolokkan, menyala, dan "
                    "kabel USB terpasang dengan benar.\n\n"
                    "Detail: " + detail + "\n\n"
                    "Buka Pengaturan → Printer untuk mengkonfigurasi ulang."
                );
            } else if (cat == PrinterError::NotConnected) {
                emit unableToPrint(
                    "Koneksi printer belum dibuka.\n\n" + detail + "\n\n"
                    "Buka Pengaturan → Printer untuk mengonfigurasi port serial."
                );
            } else if (cat == PrinterError::PortBusy || cat == PrinterError::PermissionDenied) {
                emit unableToPrint(
                    "Port serial tidak dapat dibuka.\n\n" + detail
                );
            } else if (cat == PrinterError::WriteTimeout || cat == PrinterError::WriteFailed) {
                emit unableToPrint(
                    "Gagal mengirim data ke printer.\n\n" + detail + "\n\n"
                    "Cek kabel USB dan pastikan printer dalam keadaan menyala. "
                    "Jika masalah berlanjut, cabut dan colok kembali kabel USB printer."
                );
            } else {
                emit unableToPrint(
                    "Gagal mencetak struk.\n\n" + detail
                );
            }
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
    if (m_disableAutoPrint) return;
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