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
}

ConfigureSerialPosDialog::~ConfigureSerialPosDialog()
{
    delete ui;
}

void ConfigureSerialPosDialog::on_testButton_clicked()
{
    auto &printer = PosPrinter::instance();
    if(!printer.connectSerialPort(ui->portNameBox->currentText(), ui->baudRateBox->currentData().toInt())) {
        QMessageBox::critical(this, "Koneksi Gagal", "Tidak dapat terhubung ke printer serial");
        return ;
    }
    printer.disconnectSerialPort();
    QSettings settings;
    settings.setValue("SerialPrinter/PortName", ui->portNameBox->currentText());
    settings.setValue("SerialPrinter/BaudRate", ui->baudRateBox->currentText().toInt());
    settings.sync();
}

