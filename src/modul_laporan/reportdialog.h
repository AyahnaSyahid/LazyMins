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
    enum ReportMode { SalesMode, ExpenseMode };
    ReportDialog(QSqlDatabase &db, int adminId = 1, QWidget * parent = nullptr);
    ~ReportDialog();

    void setMode(ReportMode rm);

private slots:
    void on_simpanButton_clicked();
    void on_refreshButton_clicked();

private:
    Ui::ReportDialog *ui;
    ReportLoader *loader;
    ReportMode m_reportMode;
};
