#ifndef ORDERINPUTDIALOG_H
#define ORDERINPUTDIALOG_H

#include <QDialog>

namespace Ui {
    class OrderInputDialog;
}

class OrderInputDialog : public QDialog
{
  Q_OBJECT
  public:
    explicit OrderInputDialog(QWidget *parent=nullptr);
    ~OrderInputDialog();
    inline const QString &namaBarang() const { return m_namaBarang; }
    inline const int &harga() const { return m_harga; }
    inline const int &qty() const { return m_qty; }
    inline int subTotal() const { return m_qty * m_harga; }
  
  private slots:
    void accept() override;
  
  private:
    Ui::OrderInputDialog *ui;
    QString m_namaBarang;
    int m_harga, m_qty;
};

#endif