#include "productmanager.h"
#include "createproductdialog.h"
#include "editproductdialog.h"

ProductManager::ProductManager(QObject* parent) : TableManager("products", parent) {}
ProductManager::~ProductManager(){}

QDialog* ProductManager::createDialog(QWidget* parent) {
    CreateProductDialog* cpd = new CreateProductDialog(parent);
    connect(cpd, &CreateProductDialog::accepted, this, &ProductManager::created);
    return cpd;
}

QDialog* ProductManager::editDialog(int pid, QWidget* parent) {
    EditProductDialog* epd = new EditProductDialog(pid, parent);
    connect(epd, &EditProductDialog::accepted, this, &ProductManager::modified);
    return epd;
}

void ProductManager::createProduct() {
    auto d = createDialog();
    d->open();
}

void ProductManager::editProduct(int pid) {
    auto e = editDialog(pid);
    e->open();
}
