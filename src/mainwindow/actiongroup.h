#pragma once

#include <QObject>
#include <QAction>
#include <QWidget>

class ActionGroup : public QObject
{
  Q_OBJECT
  public:
    ActionGroup(QObject * = nullptr);
    void setRootWidget(QWidget *root);

    QAction* buatAkunTransaksiAction;
    QAction* catatPengeluaranAction;

  private slots:
    void on_buatAkunTransaksiAction_triggered();
    void on_catatPengeluaranAction_triggered();

  signals:
    void newAkunTransaksiCreated();
  
  private:
    QWidget *m_root = nullptr;
};