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
    void tstCreateOrder();
    void tstCreateInvoice();
    void tstCreatePayment();
};
