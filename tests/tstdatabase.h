#pragma once

#include <QObject>
#include <QtTest>

class tstDatabase : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void tstCreateRootUser();
    void tstLoginWithRootUser();
    void tstCreateKasirUser();
    void tstChangeCurrentUser();
    void tstCreateCustomer();
    void tstCreateProduct();
    void tstItemFlowMechanism();
    void tstCreateInvoice();
    void tstCreateTransactionAccount();
    void tstPaymentMechanism();
};
