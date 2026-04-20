#pragma once

#include <QDialog>

namespace Ui {
    class PosPrinterTestDialog;
}

class PosPrinterTestDialog : public QDialog
{
  Q_OBJECT
  
  public:
    PosPrinterTestDialog(QWidget * = nullptr);
    ~PosPrinterTestDialog();
  
  private slots:
    void on_testKoneksi_clicked();
    void on_testEscPrint_clicked();
    void on_testDummyStruk_clicked();
  
  private:
    Ui::PosPrinterTestDialog *ui;
    bool m_connected_state;
};