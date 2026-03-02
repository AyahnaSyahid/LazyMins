#pragma once

#include <QMainWindow>

class MainWindow : public QMainWindow
{
  Q_OBJECT
  
  public:
    explicit MainWindow(QWidget *p=nullptr);
    ~MainWindow();
  
}