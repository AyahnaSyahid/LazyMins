#pragma once

#include <QMainWindow>

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT
  public:
    explicit MainWindow(QWidget *p=nullptr);
    ~MainWindow();
  
  public slots:
    void openLoginForm();
  
  private slots:
    void currentUserChanged();
    void on_actionLaporanPengeluaranHariIni_triggered();
    void on_actionLaporanPenjualanHariIni_triggered();

  private:

    void setupToolbarActions();
    Ui::MainWindow *ui;
};