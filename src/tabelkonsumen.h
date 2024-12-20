#ifndef TabelKonsumenH
#define TabelKonsumenH

#include <QObject>
#include <QWidget>

namespace Ui {
    class TabelKonsumen;
}
class KonsumenView;
class CustomerModel;
class TabelKonsumen : public QWidget
{
    Q_OBJECT
    
  public:
    TabelKonsumen(QWidget* = nullptr);
    ~TabelKonsumen();
    KonsumenView* view() const;
    
  private slots:
    void on_lineEdit_textChanged(const QString&);
    void headerContextMenu(const QPoint&);
    
  private:
    // CustomerModel* csm;
    Ui::TabelKonsumen* ui;
};

#endif //TabelKonsumenH