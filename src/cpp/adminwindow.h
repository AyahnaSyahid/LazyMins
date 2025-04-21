#ifndef AdminWindow_H
#define AdminWindow_H

#include <QMainWindow>

namespace Ui {
    class AdminWindow;
}

class AdminWindow : public QMainWindow
{
    Q_OBJECT

public:
    AdminWindow(QWidget* =nullptr);
    ~AdminWindow();

private:
    Ui::AdminWindow* ui;

};

#endif