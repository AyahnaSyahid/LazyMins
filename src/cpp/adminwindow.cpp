#include "adminwindow.h"
#include "../ui-files/ui_adminwindow.h"

AdminWindow::AdminWindow(QWidget* parent):
ui(new Ui::AdminWindow), QMainWindow(parent) {
    ui->setupUi(this);
}

AdminWindow::~AdminWindow() {
    delete ui;
}