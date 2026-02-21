#pragma once

#include <QDialog>

namespace Ui {
  class Konfigurasi;
}

class Konfigurasi : public QDialog
{
  Q_OBJECT
  
  public:
    explicit Konfigurasi(QWidget *p=nullptr);
    ~Konfigurasi();
  
  public slots:
    void accept() override;
  
  private slots:
    void on_pilihButton_clicked();
  
  private:
    Ui::Konfigurasi *ui;
};