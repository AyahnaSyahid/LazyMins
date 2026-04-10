#pragma once

#include <QObject>
#include <QAction>

class ActionGroup : public QObject
{
  Q_OBJECT
  public:
    ActionGroup(QObject * = nullptr);
    
    QAction* buatAkunTransaksiAction;

  private slots:
    void on_buatAkunTransaksiAction_triggered();

  signals:
    void newAkunTransaksiCreated();
};