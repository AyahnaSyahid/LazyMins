#ifndef JUNVMAINWINDOW_H
#define JUNVMAINWINDOW_H

#include <QMainWindow>

namespace Ui {
  class JINVMainWindow;
}

class JINVMainWindow : public QMainWindow
{
  Q_OBJECT
  public:
    JINVMainWindow(QWidget *parent=nullptr);
    ~JINVMainWindow();
  
  private slots:
    void openInvoiceMaker();
    void openStoreInfoEditor();
    void openInvoiceEditor(int invoice_id);
  
  private:
    void installDockable(QWidget *widget, const Qt::DockWidgetArea a, const QString& name);
    Ui::JINVMainWindow *ui;
};

#endif