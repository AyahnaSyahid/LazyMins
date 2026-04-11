#pragma once

#include "dataviewer.h"

class QAction;
class PaymentsDataViewer : public DataViewer
{
    Q_OBJECT

public:
    explicit PaymentsDataViewer(QWidget *parent = nullptr);
    ~PaymentsDataViewer();

    // Filter opsional berdasarkan invoice
    void filterByInvoiceId(int invoiceId);
    void clearInvoiceFilter();

    QAction *addPaymentAction();

public slots:
    void openCreatePaymentDialog();

private slots:
    void on_dataView_customContextMenuRequested(const QPoint &pt);
    void onAddPaymentActionTriggered();
    void openVerifyPaymentDialog(int paymentId);

private:
    Ui::DataViewer *ui;
    QAction *m_addPaymentAction = nullptr;
};
