#pragma once

#include <QDialog>
#include <QSqlDatabase>

namespace Ui
{
    class ReportDialog;
}

class ReportLoader;
class ReportDialog : public QDialog
{
    Q_OBJECT
public:
    ReportDialog(QSqlDatabase &db, int adminId = 1, QWidget * parent = nullptr);
    ~ReportDialog();

private slots:
    void on_simpanButton_clicked();
    void on_refreshButton_clicked();

private:
    Ui::ReportDialog *ui;
    ReportLoader *loader;
};
