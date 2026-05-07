#pragma once

namespace Ui {
    class InfoPercetakanDialog;
}

#include <QDialog>

struct InfoPercetakan;

class InfoPercetakanDialog : public QDialog
{
    Q_OBJECT
    public:
        explicit InfoPercetakanDialog(QWidget *parent = nullptr);
        ~InfoPercetakanDialog();
    
    private slots:
        void on_simpanButton_clicked();
        void onSaveSuccess();
        void onSaveFailed(const QString& msg);
        void saveInfoPercetakan(const InfoPercetakan& info);

    signals:
        void trySave(const InfoPercetakan& info);
        void saveSuccess();
        void saveFailed(const QString& msg);

    private:
        Ui::InfoPercetakanDialog *ui;
};