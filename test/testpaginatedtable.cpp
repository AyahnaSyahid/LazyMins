#include <QApplication>
#include <QStandardItemModel>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QHeaderView>
#include <QTableView>

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QLabel>

#include "paginatedproxymodel.h"
#include "paginatedtableview.h"

QWidget* setButton(PaginatedProxyModel* md) {
    auto ptr = new QWidget;
    auto lay = new QVBoxLayout;
    auto limitSetterLayout = new QHBoxLayout;
    auto lab = new QLabel("Set Limit", ptr);
    auto limitEdit = new QSpinBox(ptr);
    limitEdit->setMaximum(1000);
    auto setB = new QPushButton("Set", ptr);
    limitSetterLayout->addWidget(lab);
    limitSetterLayout->addWidget(limitEdit);
    limitSetterLayout->addWidget(setB);
    lay->addLayout(limitSetterLayout);
    
    QObject::connect(setB, &QPushButton::clicked, [limitEdit, md]{ md->changeLimit(limitEdit->value()); });
    QObject::connect(md, &PaginatedProxyModel::pageChanged, [lab, md](int p){
      lab->setText(QString("Showing %1 of %2").arg(p).arg(md->maxPage()));} );
    
    auto b = new QPushButton("First", ptr);
    QObject::connect(b, &QPushButton::clicked, md, &PaginatedProxyModel::loadFirst);
    lay->addWidget(b);
    b = new QPushButton("Prev", ptr);
    QObject::connect(b, &QPushButton::clicked, md, &PaginatedProxyModel::loadPrev);
    lay->addWidget(b);
    b = new QPushButton("Next", ptr);
    QObject::connect(b, &QPushButton::clicked, md, &PaginatedProxyModel::loadNext);
    lay->addWidget(b);
    b = new QPushButton("Last", ptr);
    QObject::connect(b, &QPushButton::clicked, md, &PaginatedProxyModel::loadLast);
    lay->addWidget(b);
    ptr->setLayout(lay);
    ptr->adjustSize();
    return ptr;
}

int main(int argc, char** args) {
  QApplication app(argc, args);
  auto base = QSqlDatabase::addDatabase("QSQLITE");
  base.setDatabaseName(R"x(C:\Users\A3Plus\QTAPP\data\a3p.db)x");
  base.open();
  QSqlQueryModel qm;
  qm.setObjectName("query model");
  qm.setQuery("SELECT * FROM a3pdata");
  PaginatedTableView tv;
  tv.setLimit(5000);
  // PaginatedProxyModel ppm(0, 10000);
  // ppm.setSourceModel(&qm);
  tv.setModel(&qm);
  tv.verticalHeader()->setMinimumSectionSize(20);
  tv.verticalHeader()->setDefaultSectionSize(20);
  // QWidget* ctl = setButton(&ppm);
  // ctl->show();
  tv.show();
  app.exec();
}