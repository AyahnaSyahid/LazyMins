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

private slots:
    void on_testButton_clicked();

private:
    Ui::ConfigureSerialPosDialog *ui;
};