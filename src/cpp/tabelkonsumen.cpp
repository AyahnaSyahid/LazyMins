#include "tabelkonsumen.h"
#include "../ui/ui_tabelkonsumen.h"
#include "customermodel.h"
#include <QSqlQuery>
#include <QtDebug>
#include <QMenu>
#include <QAction>


TabelKonsumen::TabelKonsumen(QWidget* parent)
  :ui(new Ui::TabelKonsumen), QWidget(parent)
{
    ui->setupUi(this);
    ui->tableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableView->hideColumn(0);
    ui->tableView->hideColumn(2);
    ui->tableView->hideColumn(3);
    ui->tableView->hideColumn(5);
    ui->tableView->hideColumn(6);
    auto hv = ui->tableView->horizontalHeader();
    connect(hv, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(headerContextMenu(QPoint)));
}

TabelKonsumen::~TabelKonsumen()
{
    delete ui;
}

void TabelKonsumen::on_lineEdit_textChanged(const QString& txt)
{
    QString filter("customer_name || ' ' || email || ' ' || phone_number || ' ' || address || ' ' || email LIKE '%%1%'");
    if(!txt.isEmpty()) {
        ui->tableView->setModelFilter(filter.arg(txt));
        // qDebug() << csm->query().lastQuery();
    } else {
        ui->tableView->setModelFilter("");
    }
}

void TabelKonsumen::headerContextMenu(const QPoint& p)
{
    auto model = ui->tableView->model();
    if( (! model) || ui->tableView->model()->columnCount() < 1)
        return;
    QMenu menu;
    for(int nc = 0; nc < model->columnCount(); ++nc)
    {
        auto act = menu.addAction(model->headerData(nc, Qt::Horizontal).toString());
        act->setCheckable(true);
        act->setChecked(!ui->tableView->horizontalHeader()->isSectionHidden(nc));
        act->setProperty("CS", nc);
        // connect(act, &QAction::toggled, this, &TabelKonsumen::actReceiver);
    }
    auto trig = menu.exec(ui->tableView->mapToGlobal(p));
    if(trig) {
        int col = trig->property("CS").toInt();
        if(trig->isChecked()) {
            ui->tableView->showColumn(col);
        } else {
            ui->tableView->hideColumn(col);
        }
    }
}

KonsumenView* TabelKonsumen::view() const{
    return ui->tableView;
}