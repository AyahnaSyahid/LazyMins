#ifndef CREATEINVOICEDIALOG_H
#define CREATEINVOICEDIALOG_H

#include <QDialog>
#include <QResizeEvent>
#include <QStandardItemModel>

#include "../invoicedatatype.h"

namespace Ui {
  class CreateInvoiceDialog;
};

class CreateInvoiceDialog : public QDialog {
  Q_OBJECT
  public:
    static void RegisterMetaType();
    explicit CreateInvoiceDialog(QWidget *parent=nullptr);
    ~CreateInvoiceDialog();
  
  private slots:
    void on_tableActionInsert_triggered();
    void on_tableActionDelete_triggered();
    void on_spinBoxBayar_valueChanged(int);
    void on_simpanButton_clicked();
    void inputDialogAccepted();
    void notaDataChanged(const QModelIndex& t, const QModelIndex& b, const QList<int> &roles);
    void notaModelRowCountChanged();
    void onNotaSaveDone(bool);

  protected:
    void resizeEvent(QResizeEvent *re) override;
    void showEvent(QShowEvent *se) override;
  
  signals:
    void saveNotaRequest(const InvoiceData &ida);
  
  private:
    int totalPrice() const;
    Ui::CreateInvoiceDialog *ui;
    QStandardItemModel *notaModel;    
};

Q_DECLARE_METATYPE(InvoiceData::ItemData);
Q_DECLARE_METATYPE(InvoiceData);
#endif