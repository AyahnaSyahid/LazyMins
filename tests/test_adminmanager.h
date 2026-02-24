#pragma once

#include <QtTest>
#include <QObject>

class test_AdminManager : public QObject
{
  Q_OBJECT

  QList<QVariantMap> users;

  private slots:
    void initTestCase();
    void testCreate();
};