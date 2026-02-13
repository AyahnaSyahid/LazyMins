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
  
  private:
    Ui::JINVMainWindow *ui;
} 

#endif