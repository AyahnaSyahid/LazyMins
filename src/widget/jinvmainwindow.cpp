#include "jinvmainwindow.h"
#include "ui_jinvmainwindow.h"

JINVMainWindow::JINVMainWindow(QWidget *p) :
  ui(new Ui::JINVMainWindow), QMainWindow(p)
{
  ui->setupUi(this);
}

JINVMainWindow::JINVMainWindow() { delete ui; }