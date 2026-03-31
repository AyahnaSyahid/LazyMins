#include <QDialog>

namespace Ui {
  class SetupWindow;
}

class SetupWindow : public QDialog
{
  Q_OBJECT

  public:
    SetupWindow(QWidget *parent=nullptr);
    ~SetupWindow();
  
  private slots:
    void on_installButton_clicked();
    void on_browseButton_clicked();
  
  signals:
    void setupFinished();
    void setupFailed();

  private:
    Ui::SetupWindow *ui;
    bool m_abort;
};