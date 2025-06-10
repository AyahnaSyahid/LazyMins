#ifndef InvoiceViewDialog_H
#define InvoiceViewDialog_H

namespace Ui {
    class InvoiceViewDialog;
}

#include <QDialog>
#include <QLocale>

class QSqlQueryModel;
class InvoiceViewDialog : public QDialog {
    Q_OBJECT

public:
    explicit InvoiceViewDialog(int c_id, QWidget* =nullptr);
    ~InvoiceViewDialog();

private:
    QSqlQueryModel* model;
    Ui::InvoiceViewDialog* ui;
    // QLocale loc;
};

#endif