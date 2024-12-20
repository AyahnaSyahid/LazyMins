#include "addexpensedialog.h"
#include "ui_addexpensedialog.h"
#include "notifier.h"

#include "expenseitem.h"
#include "divisionitem.h"
#include "helper.h"

#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QDateTime>
#include <QLineEdit>
#include <QPushButton>
#include <QInputDialog>
#include <QCompleter>

AddExpenseDialog::AddExpenseDialog(QWidget* parent)
: ui(new Ui::AddExpenseDialog), QDialog(parent) {
    ui->setupUi(this);
    connect(ui->pushButton2, &QPushButton::clicked, this, &AddExpenseDialog::accept);
    auto qmodel = new QSqlQueryModel(this);
    qmodel->setObjectName("divisionsModel");
    qmodel->setQuery("SELECT division_id, division_name FROM divisions");
    ui->comboBox->setModel(qmodel);
    ui->comboBox->setModelColumn(1);
    ui->comboBox->setCurrentIndex(-1);
    ui->comboBox->completer()->setCompletionMode(QCompleter::PopupCompletion);
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
}

AddExpenseDialog::~AddExpenseDialog() {
    delete ui;
}

void AddExpenseDialog::on_pushButton_clicked() {
    bool ok = false;
    auto tx = QInputDialog::getText(this, "Tambahkan Divisi baru", "Nama Divisi", QLineEdit::Normal, QString(), &ok);
    if(ok) {
        if (!tx.trimmed().isEmpty()) {
            DivisionItem di(tx.trimmed());
            if(di.save()) {
                auto qmodel = qobject_cast<QSqlQueryModel*>(ui->comboBox->model());
                qmodel->setQuery(qmodel->query().lastQuery());
                dbNot->emitChanged("divisions");
            } else {
                MessageHelper::information(this, "Kesalahan", "Gagal menyimpan data");
                return;
            }
        } else {
            MessageHelper::information(this, "Periksa masukan", "Anda belum mengisi nama Divisi");
            ui->pushButton->click();
        }
    }
}

void AddExpenseDialog::accept() {
    if(ui->comboBox->currentIndex() < 0) {
        ToolTipHelper::showTip(mapToGlobal(ui->comboBox->geometry().topLeft()), "Pastikan nama divisi sudah valid");
        return;
    }
    if(ui->spinBox->value() < 1) {
        ToolTipHelper::showTip(mapToGlobal(ui->spinBox->geometry().topLeft()), "Jumlah uang tidak valid");
        return;
    }
    if(ui->lineEdit->text().trimmed().isEmpty()) {
        ToolTipHelper::showTip(mapToGlobal(ui->lineEdit->geometry().topLeft()), "<p><b>Kategori</b><p> harus diisi");
        return;
    }
    if(ui->plainTextEdit->toPlainText().trimmed().isEmpty()) {
        ToolTipHelper::showTip(mapToGlobal(ui->lineEdit->geometry().topLeft()), "<p><b><i>Deskripsi</i></b> akan memudahkan anda mencari petunjuk pengeluaran nantinya");
        return;
    }
    
    DivisionItem divi(ui->comboBox->currentText());
    if(divi.division_id < 1) {
        ToolTipHelper::showTip(mapToGlobal(ui->comboBox->geometry().topLeft()), "Pastikan nama divisi sudah valid");
        return;
    }
    
    ExpenseItem exp;
    exp.amount = ui->spinBox->value();
    exp.division_id = divi.division_id;
    exp.description = ui->plainTextEdit->toPlainText().trimmed();
    exp.category = ui->lineEdit->text().trimmed();
    exp.expense_date = ui->dateTimeEdit->dateTime();
    exp.created_by = LoginNotifier::instance()->currentUser().user_id;
    if(!exp.save()) {
        MessageHelper::information(this, "Kesalahan", "Tidak dapat menyimpan data Pengeluaran");
        return;
    }
    dbNot->emitChanged("expenses");
    QDialog::accept();
}

EditExpenseDialog::EditExpenseDialog(qint64 e_id, QWidget* parent)
: expense_id(e_id), AddExpenseDialog(parent) {
    ExpenseItem exp(e_id);
    DivisionItem divi(exp.division_id);
    ui->comboBox->setCurrentIndex(ui->comboBox->findText(divi.division_name));
    ui->dateTimeEdit->setDateTime(exp.expense_date);
    ui->spinBox->setValue(exp.amount);
    ui->lineEdit->setText(exp.category);
    ui->plainTextEdit->setPlainText(exp.description);
}

void EditExpenseDialog::accept() {
    if(ui->comboBox->currentIndex() < 0) {
        ToolTipHelper::showTip(mapToGlobal(ui->comboBox->geometry().topLeft()), "Pastikan nama divisi sudah valid");
        return;
    }
    if(ui->spinBox->value() < 1) {
        ToolTipHelper::showTip(mapToGlobal(ui->spinBox->geometry().topLeft()), "Jumlah uang tidak valid");
        return;
    }
    if(ui->lineEdit->text().trimmed().isEmpty()) {
        ToolTipHelper::showTip(mapToGlobal(ui->lineEdit->geometry().topLeft()), "<p><b>Kategori</b><p> harus diisi");
        return;
    }
    if(ui->plainTextEdit->toPlainText().trimmed().isEmpty()) {
        ToolTipHelper::showTip(mapToGlobal(ui->lineEdit->geometry().topLeft()), "<p><b><i>Deskripsi</i></b> akan memudahkan anda mencari petunjuk pengeluaran nantinya");
        return;
    }
    
    DivisionItem divi(ui->comboBox->currentText());
    if(divi.division_id < 1) {
        ToolTipHelper::showTip(mapToGlobal(ui->comboBox->geometry().topLeft()), "Pastikan nama divisi sudah valid");
        return;
    }
    
    ExpenseItem exp(expense_id);
    exp.amount = ui->spinBox->value();
    exp.division_id = divi.division_id;
    exp.description = ui->plainTextEdit->toPlainText().trimmed();
    exp.category = ui->lineEdit->text().trimmed();
    exp.expense_date = ui->dateTimeEdit->dateTime();
    exp.modified_by = LoginNotifier::instance()->currentUser().user_id;
    exp.modified_date = QDateTime::currentDateTime();
    if(!exp.save()) {
        MessageHelper::information(this, "Kesalahan", "Tidak dapat menyimpan update data pengeluaran");
        return;
    }
    dbNot->emitChanged("expenses");
    QDialog::accept();
}