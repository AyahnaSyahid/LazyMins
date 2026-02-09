#ifndef CREATEINVOICEDIALOG_H
#define CREATEINVOICEDIALOG_H

#include <QDialog>
#include <QResizeEvent>
#include <QStandardItemModel>

namespace Ui {
  class CreateInvoiceDialog;
};

class CreateInvoiceDialog : public QDialog {
  Q_OBJECT
  public:
    explicit CreateInvoiceDialog(QWidget *parent=nullptr);
    ~CreateInvoiceDialog();
  
  private slots:
    void on_tableActionInsert_triggered();
    void on_tableActionDelete_triggered();
    void on_spinBoxBayar_valueChanged(int);
    void inputDialogAccepted();
    void notaDataChanged(const QModelIndex& t, const QModelIndex& b, const QList<int> &roles);

  protected:
    void resizeEvent(QResizeEvent *re) override;
    void showEvent(QShowEvent *se) override;
  
  private:
    Ui::CreateInvoiceDialog *ui;
    QStandardItemModel *notaModel;
};

#endif