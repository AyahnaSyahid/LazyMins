#include "storeinfoeditordialog.h"
#include "../databaseinterface.h"
#include <QFormLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>

StoreInfoEditorDialog::StoreInfoEditorDialog(QWidget *parent) : cachedStoreInfo {}, QDialog(parent) {
    auto vl = new QVBoxLayout(this); // Langsung set parent di constructor layout
    auto fl = new QFormLayout();
    
    le1 = new QLineEdit(this);
    le2 = new QLineEdit(this);
    le3 = new QLineEdit(this);

    // QFormLayout bisa membuat QLabel internal secara otomatis
    fl->addRow("Nama Toko", le1);
    fl->addRow("Alamat", le2);
    fl->addRow("Kontak", le3);

    auto hl = new QHBoxLayout();
    auto pb = new QPushButton("Simpan", this);
    connect(pb, &QPushButton::clicked, this, &StoreInfoEditorDialog::accept);
    hl->addStretch(1);
    hl->addWidget(pb);

    vl->addLayout(fl, 1);
    vl->addLayout(hl, 0);
    setWindowTitle("Ubah parameter toko");
    // setLayout(vl) tidak wajib lagi jika 'this' sudah ada di constructor vl
    adjustSize();
    setFixedSize(sizeHint()); // sizeHint() seringkali lebih akurat untuk dialog baru
    initData();
}

StoreInfoEditorDialog::~StoreInfoEditorDialog() {}

void StoreInfoEditorDialog::initData() {
  auto db = DatabaseInterface::instance().database();
  auto optC = DatabaseInterface::instance().getStoreInfo(db);
  cachedStoreInfo = *optC;
  le1->setText(cachedStoreInfo.storeName);
  le2->setText(cachedStoreInfo.storeAddr);
  le3->setText(cachedStoreInfo.storePhone);
}

void StoreInfoEditorDialog::accept() {
  QString n = le1->text().trimmed(),
          a = le2->text().trimmed(),
          p = le3->text().trimmed();
  if (n.size() <= 0 || a.size() <= 0 || p.size() <= 0) {
    QMessageBox::information(this, "Info Pengaturan", "Semua parameter harus diisi");
    return;
  }
  StoreInfoData sid { n, a, p };
  bool saved = DatabaseInterface::instance().saveStoreInfoData(sid);
  if(!saved) {
    QMessageBox::information(this, "Info Pengaturan", "Gagal disimpan");
    initData();
    return ;
  }
  QDialog::accept();
}
