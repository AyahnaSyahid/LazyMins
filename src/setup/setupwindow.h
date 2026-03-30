#include <QMainWindow>

namespace Ui {
  class SetupWindow;
}

class SetupWindow : public QMainWindow
{
  Q_OBJECT

  public:
    SetupWindow(QWidget *parent=nullptr);
    ~SetupWindow();
  
  private slots:
    void on_installButton_clicked();
    void on_browseButton_clicked();
  
  private:
    Ui::SetupWindow *ui;
};