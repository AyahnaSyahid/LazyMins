#ifndef CSVProductUpdaterDialog_H
#define CSVProductUpdaterDialog_H

namespace Ui {
    class CSVProductUpdaterDialog;
}

#include <QDialog>
class QStandardItemModel;
class CSVProductUpdaterDialog : public QDialog {
    Q_OBJECT
public:
    CSVProductUpdaterDialog(QWidget* =nullptr);
    ~CSVProductUpdaterDialog();

private slots:
    void on_pushButton_clicked();
    void on_pushButton2_clicked();
    void on_actionDeleteSelected_triggered();
    void loadFile(const QString&, const char sep=';');
    
private:
    Ui::CSVProductUpdaterDialog* ui;
    QStandardItemModel* iMod;
};

#endif