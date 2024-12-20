#ifndef MainWindow_H
#define MainWindow_H

namespace Ui {
  class MainWindow;
}

#include <QMainWindow>

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  MainWindow(QWidget* = nullptr);
  ~MainWindow();

private slots:
  void on_actionManager_triggered();
  void on_actionEditBio_triggered();
  void on_actionMasuk_triggered();
  void userLoggedIn(qint64);
  void userLoggedOut(qint64);

private:
  Ui::MainWindow* ui;
};

#endif