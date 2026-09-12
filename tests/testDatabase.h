#pragma once
#include <QObject>
#include <QtTest>

class TestDatabase: public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void testDatabaseMigrate();
};