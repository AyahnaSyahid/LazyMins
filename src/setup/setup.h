#pragma once

#include <QObject>

class Setup : public QObject
{
  Q_OBJECT
  public:
    explicit Setup(QObject *parent=nullptr);
    ~Setup();
};