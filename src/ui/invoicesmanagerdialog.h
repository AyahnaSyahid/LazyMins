#ifndef InvoicesManagerDialog_H
#define InvoicesManagerDialog_H

namespace Ui {
    class InvoicesManagerDialog;
}

#include <QDialog>

class Database;
class InvoicesManagerModel;
class InvoicesManagerDialog : public QDialog{
    Q_OBJECT
public:
    explicit InvoicesManagerDialog(Database*, QWidget* =nullptr);
    ~InvoicesManagerDialog();

private:
    Database* db;
    Ui::InvoicesManagerDialog* ui;
    InvoicesManagerModel* inModel;
};

#endif