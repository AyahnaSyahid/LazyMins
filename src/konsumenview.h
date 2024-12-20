#ifndef KonsumenViewH
#define KonsumenViewH

#include <QTableView>

class CustomerModel;
class KonsumenView : public QTableView
{
    Q_OBJECT
  
  public:
    KonsumenView(QWidget* =nullptr);
    ~KonsumenView();
    
  public slots:
    void setModelFilter(const QString& s);
    void modelSelect();
  
  private slots:
    void customContextMenu(const QPoint& p);
  
  protected:
    using QTableView::setModel;
  
  private:
    CustomerModel* cmodel;
  
  signals:
    void manageCustomerOrder(const QVariant&);
};

#endif