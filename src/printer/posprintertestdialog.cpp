#include "posprintertestdialog.h"
#include "ui_posprintertestdialog.h"

#include "posprinter.h"
#include "src/managers/helpers.h"

#include <QMessageBox>
#include <QDateTime>
#include <QDate>
#include <QTime>

namespace {
  auto &localPP = PosPrinter::instance();
}


PosPrinterTestDialog::PosPrinterTestDialog(QWidget *p) :
ui(new Ui::PosPrinterTestDialog), QDialog(p)
{
  ui->setupUi(this);
  
  connect(ui->resetLog, &QPushButton::clicked, ui->logTextEdit, &QPlainTextEdit::clear);
  
  for(auto const& port : localPP.availableSerialPorts()) {
    ui->portCombo->addItem(port, port);
  }
  
  
  QList<int> bauds { 4800, 9600, 19200, 38400, 57600, 115200 };
  for(auto const& baud : bauds) {
    ui->baudCombo->addItem(QString::number(baud), baud);
  }
  
  
  m_connected_state = false;
  if (localPP.isSerialConnected()) {
    ui->portCombo->setCurrentIndex(ui->portCombo->findText(localPP.serialPortName()));
    ui->baudCombo->setCurrentIndex(ui->baudCombo->findText(QString::number(localPP.serialBaudRate())));
    ui->connectIndicator->setText("Connected");
    ui->testKoneksi->setText("Disconnect");
    m_connected_state = true;
  } else {
    ui->baudCombo->setCurrentIndex(-1);
    ui->portCombo->setCurrentIndex(-1);  
  }
  
  setAttribute(Qt::WA_DeleteOnClose);
}

PosPrinterTestDialog::~PosPrinterTestDialog() { delete ui; }

void PosPrinterTestDialog::on_testKoneksi_clicked() {
    if (ui->baudCombo->currentIndex() < 0 || ui->portCombo->currentIndex() < 0) {
        QMessageBox::warning(this, "Prosedur salah", "Tentukan Port COM & Baud Rate sebelum Uji Konektivitas");
        return;
    }

    QString portName = ui->portCombo->currentText();
    int baudRate = ui->baudCombo->currentData().toInt();

    if (!m_connected_state) {
        ui->logTextEdit->appendPlainText(QString("Mencoba koneksi ke %1 @ %2 baud...").arg(portName).arg(baudRate));

        if (localPP.connectSerialPort(portName, baudRate)) {
            m_connected_state = true;
            ui->testKoneksi->setText("Disconnect");
            ui->connectIndicator->setText("Connected");
            ui->logTextEdit->appendPlainText("=> Koneksi BERHASIL.");
        } else {
            ui->connectIndicator->setText("Failed");
            ui->logTextEdit->appendPlainText("=> Koneksi GAGAL: " + localPP.lastError());
            QMessageBox::critical(this, "Koneksi Gagal", localPP.lastError());
        }
    } else {
        ui->logTextEdit->appendPlainText("Memutus koneksi serial...");
        
        if (localPP.disconnectSerialPort()) {
            m_connected_state = false;
            ui->testKoneksi->setText("Connect");
            ui->connectIndicator->setText("Disconnected");
            ui->logTextEdit->appendPlainText("=> Koneksi TERPUTUS.");
        } else {
            ui->logTextEdit->appendPlainText("=> Gagal memutus koneksi: " + localPP.lastError());
            QMessageBox::warning(this, "Disconnect Gagal", localPP.lastError());
        }
    }
}

void PosPrinterTestDialog::on_testEscPrint_clicked() {
    if (!m_connected_state) {
        ui->logTextEdit->appendPlainText("Error: Printer belum terhubung.");
        QMessageBox::warning(this, "Peringatan", "Silakan hubungkan printer terlebih dahulu.");
        return;
    }

    ui->logTextEdit->appendPlainText("Mengirim perintah Test Print ESC/POS...");
    
    if (localPP.testPrintViaEscPos()) {
        ui->logTextEdit->appendPlainText("=> Test Print BERHASIL dikirim.");
    } else {
        ui->logTextEdit->appendPlainText("=> Test Print GAGAL: " + localPP.lastError());
        QMessageBox::critical(this, "Gagal", "Gagal melakukan Test Print:\n" + localPP.lastError());
    }
}

void PosPrinterTestDialog::on_testDummyStruk_clicked() {
    if (!m_connected_state) {
        ui->logTextEdit->appendPlainText("Error: Printer belum terhubung.");
        QMessageBox::warning(this, "Peringatan", "Silakan hubungkan printer terlebih dahulu.");
        return;
    }

    ui->logTextEdit->appendPlainText("Menyiapkan data Dummy Struk...");

    // Setup Dummy Data
    Receipt dummy;
    
    auto res = DBOperationHelper::loadInvoiceDataFast(6, &dummy);
    
    if (!res.ok) {
      QMessageBox::warning(this, "Peringatan", res.error);
      ui->logTextEdit->appendPlainText("Gagal memuat invoice data" + res.error);
      return ;
    }
    
    
    // Log isi struk singkat
    ui->logTextEdit->appendPlainText(QString("Detail Invoice: %1 | Total: Rp%2")
                                     .arg(dummy.invoiceNo)
                                     .arg(dummy.grandTotal));
    ui->logTextEdit->appendPlainText("Mengirim data struk ke printer...");
    
    qDebug() << dummy;
    for(auto const& item : dummy.items) {
      qDebug() << item;
      for (auto const& fin : item.finishings) {
        qDebug() << fin;
      }
    }
    // Eksekusi Cetak
    if (localPP.printReceiptViaEscPos(dummy)) {
        ui->logTextEdit->appendPlainText("=> Dummy Struk BERHASIL dicetak.");
    } else {
        ui->logTextEdit->appendPlainText("=> Dummy Struk GAGAL: " + localPP.lastError());
        QMessageBox::critical(this, "Gagal", "Gagal mencetak struk dummy:\n" + localPP.lastError());
    }
}