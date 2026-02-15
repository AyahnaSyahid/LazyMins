#ifndef STOREINFOEDITORDIALOH_H
#define STOREINFOEDITORDIALOG_H

#include <QDialog>
#include <QLineEdit>

#include "../invoicedatatype.h"

class StoreInfoEditorDialog : public QDialog
{
  public:
    StoreInfoEditorDialog(QWidget *parent);
    ~StoreInfoEditorDialog();
  
  private slots:
    void accept() override;
    void initData();
  private:
    QLineEdit *le1, *le2, *le3;
    StoreInfoData cachedStoreInfo;
};

#endif