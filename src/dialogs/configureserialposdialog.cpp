#include "configureserialposdialog.h"
#include "ui_configureserialposdialog.h"

#include "src/printer/posprinter.h"
#include <QMessageBox>
#include <QSettings>

ConfigureSerialPosDialog::ConfigureSerialPosDialog(QWidget *parent) :
QDialog(parent), ui(new Ui::ConfigureSerialPosDialog)
{
    ui->setupUi(this);
    ui->portNameBox->clear();
    
    auto &inst = PosPrinter::instance();
    for(auto const &portName : inst.availableSerialPorts())
        ui->portNameBox->addItem(portName);

    ui->baudRateBox->clear();
    for(auto const& baudRate : QList<int>{ 4800, 9600, 19200, 38400, 57600, 115200 })
        ui->baudRateBox->addItem(QString::number(baudRate), baudRate);

    // Tombol simpan dinonaktifkan sampai tes koneksi berhasil dilakukan
    ui->simpanButton->setEnabled(false); 
}

ConfigureSerialPosDialog::~ConfigureSerialPosDialog()
{
    delete ui;
}

bool ConfigureSerialPosDialog::hasValidConfig()
{
    QSettings settings;
    QString port = settings.value("SerialPrinter/PortName").toString();
    int baud = settings.value("SerialPrinter/BaudRate").toInt();
    
    // Konfigurasi dianggap valid jika port tidak kosong dan baud rate terdefinisi
    return !port.isEmpty() && baud > 0;
}

void ConfigureSerialPosDialog::on_testButton_clicked()
{
    auto &printer = PosPrinter::instance();
    QString port = ui->portNameBox->currentText();
    int baud = ui->baudRateBox->currentData().toInt();

    if(!printer.connectSerialPort(port, baud)) {
        QMessageBox::critical(this, "Koneksi Gagal", "Tidak dapat terhubung ke printer serial. Pastikan kabel terpasang dan port benar.");
        ui->simpanButton->setEnabled(false);
        return;
    }

    // Jika koneksi berhasil, aktifkan tombol simpan
    ui->simpanButton->setEnabled(true);
    
    auto ask = QMessageBox::question(this, "Koneksi Berhasil", 
                                    "Koneksi berhasil, simpan dan gunakan printer ini?", 
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    
    if (ask == QMessageBox::Yes) {
        saveSettings();
        accept(); // Tutup dialog dengan hasil QDialog::Accepted
    }
}

void ConfigureSerialPosDialog::on_simpanButton_clicked()
{
    saveSettings();
    accept();
}

void ConfigureSerialPosDialog::saveSettings()
{
    QSettings settings;
    settings.setValue("SerialPrinter/PortName", ui->portNameBox->currentText());
    // Mengambil data integer dari baudRateBox
    settings.setValue("SerialPrinter/BaudRate", ui->baudRateBox->currentData().toInt()); 
    settings.sync();
}