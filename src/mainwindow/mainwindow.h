#pragma once

#include <QMainWindow>

namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT
  public:
    explicit MainWindow(QWidget *p=nullptr);
    ~MainWindow();
  
  private slots:
    
  
  private:
    void setupToolbarActions();
    Ui::MainWindow *ui;
};