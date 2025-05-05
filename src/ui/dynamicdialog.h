#ifndef DynamicDialog_H
#define DynamicDialog_H

#include <QDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QSqlField>
#include <QMessageBox>
#include <QVBoxLayout>

class InsertDialog : public QDialog {
    Q_OBJECT
public:
    InsertDialog(QSqlTableModel *model, QWidget *parent = nullptr);
private slots:
    void onOkClicked();

signals:
    void modified();

private:
    QSqlTableModel *model;
    QMap<QString, QLineEdit*> inputs; // Menyimpan input widget berdasarkan nama field
    QStringList requiredFields;       // Daftar field yang wajib diisi
};

// Untuk file yang berisi Q_OBJECT jika tidak ada header terpisah

// Contoh penggunaan:
// int main(int argc, char *argv[]) {
//     QApplication app(argc, argv);
//     QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
//     db.setDatabaseName("database.db");
//     db.open();
//     QSqlTableModel model;
//     model.setTable("orders");
//     model.select();
//     InsertDialog dialog(&model);
//     dialog.exec();
//     return app.exec();
// }


#endif