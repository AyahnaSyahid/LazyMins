#pragma once

#include <QDialog>

namespace Ui {
    class ConfigureSerialPosDialog;
}

class ConfigureSerialPosDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigureSerialPosDialog(QWidget *parent = nullptr);
    ~ConfigureSerialPosDialog();

    // Fungsi statis untuk mengecek apakah konfigurasi sudah ada dan valid
    static bool hasValidConfig(); 

private slots:
    void on_testButton_clicked();
    void on_simpanButton_clicked();

private:
    void saveSettings();
    Ui::ConfigureSerialPosDialog *ui;
};