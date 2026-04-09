#pragma once

#include <QDialog>
#include "src/managers/managers.h"

namespace Ui {
  class KategoriProdukDialog;
}

class KategoriProdukDialog : public QDialog
{
  
  Q_OBJECT

public:
  explicit KategoriProdukDialog(QWidget *pa=nullptr);
  ~KategoriProdukDialog();

  void prepareCreate();
  void prepareModify(int _id);

  bool isInputAcceptable() const;
  
private slots:
  void on_simpanButton_clicked();

// Signals QDialog::accepted berarti update / insert berhasil

private:
  Ui::KategoriProdukDialog *ui;
  ProductCategoryManager pcm;
  int m_id;
};