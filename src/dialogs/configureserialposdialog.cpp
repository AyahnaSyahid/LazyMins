#include "configureserialposdialog.h"
#include "ui_configureserialposdialog.h"

#include "src/printer/posprinter.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QSettings>

ConfigureSerialPosDialog::ConfigureSerialPosDialog(QWidget *parent) :
QDialog(parent), ui(new Ui::ConfigureSerialPosDialog)
{
    ui->setupUi(this);
    ui->portNameBox->clear();
    for(auto const &info : QSerialPortInfo::availablePorts())
        ui->portNameBox->addItem(info.portName(), info.portName());
    ui->baudRateBox->clear();
    for(auto const &baudRate : QSerialPortInfo::standardBaudRates())
        ui->baudRateBox->addItem(QString::number(baudRate), baudRate);
}

ConfigureSerialPosDialog::~ConfigureSerialPosDialog()
{
    delete ui;
}

void ConfigureSerialPosDialog::on_testButton_clicked()
{
    auto &printer = PosPrinter::instance();
    if(!printer.connectSerialPort(ui->portNameBox->currentText(), ui->baudRateBox->currentText().toInt())) {
        QMessageBox::critical(this, "Koneksi Gagal", "Tidak dapat terhubung ke printer serial");
    }
    printer.disconnectSerialPort();
    QSettings settings;
    settings.setValue("SerialPrinter/PortName", ui->portNameBox->currentText());
    settings.setValue("SerialPrinter/BaudRate", ui->baudRateBox->currentText().toInt());
    settings.sync();
}

