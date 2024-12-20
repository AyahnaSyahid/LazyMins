#include "expensesdockwidgetcontents.h"
#include "ui_expensesdockwidgetcontents.h"
#include "notifier.h"
#include <QtDebug>
#include <QtMath>

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlQueryModel>
#include <QStringListModel>
#include <QSortFilterProxyModel>
#include <QModelIndex>
#include <QVariant>
#include <QLocale>
#include <QDateTime>
#include <QPoint>
#include <QMenu>
#include <QAction>
#include <QHeaderView>

const QString baseQuery(R"-(
SELECT expense_id AS ID,
       expense_date AS Tanggal,
       division_name AS Divisi,
       amount As Jumlah,
       expenses.category AS Kategori,
       expenses.description AS Deskripsi,
       case when creator.full_name is null then creator.username else creator.full_name END AS Pencatat,
       expenses.created_date AS Dibuat,
       expenses.modified_date AS Diubah,
       CASE WHEN modif.full_name IS NULL THEN modif.username ELSE modif.full_name END AS Pengubah
  FROM expenses,
       divisions USING ( division_id ),
       users AS creator ON creator.user_id = created_by
LEFT JOIN users AS modif ON modif.user_id = modified_by
)-");

class ExpensesProxyModel : public QSortFilterProxyModel {
public:
    ExpensesProxyModel(QObject* p = nullptr) : QSortFilterProxyModel(p) {}
    QVariant data(const QModelIndex& mi, int role=Qt::DisplayRole) const;
};

class MonthModel : public QSqlQueryModel {
public:
    MonthModel(QObject *parent=nullptr);
    QVariant data(const QModelIndex& mi, int =Qt::DisplayRole) const override;
};

ExpensesDockWidgetContents::ExpensesDockWidgetContents(QWidget* parent)
: ui(new Ui::ExpensesDockWidgetContents), QWidget(parent) {
    ui->setupUi(this);
    modelBulanan = new MonthModel(this);
    modelTahunan = new QSqlQueryModel(this);
    qobject_cast<QSqlQueryModel*>(modelTahunan)->setQuery("SELECT DISTINCT strftime('%Y', datetime(expense_date)) AS tahun FROM expenses");
    modelExpenses = new QSqlQueryModel(this);
    connect(ui->lineEdit, SIGNAL(textChanged(QString)), this, SLOT(refreshModel()));
    ui->comboBulan->setModel(modelBulanan);
    ui->comboBulan->setModelColumn(1);
    ui->comboTahun->setModel(modelTahunan);
    auto proxy = new ExpensesProxyModel(this);
    proxy->setSourceModel(modelExpenses);
    ui->tableView->setModel(proxy);
    ui->tableView->hideColumn(0);
    ui->tableView->hideColumn(6);
    ui->tableView->hideColumn(7);
    ui->tableView->hideColumn(8);
    ui->tableView->hideColumn(9);
    auto hhead = ui->tableView->horizontalHeader();
    hhead->setContextMenuPolicy(Qt::CustomContextMenu);
    hhead->setSectionsMovable(true);
    connect(hhead, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(horizontalHeaderContextMenu(QPoint)));
    connect(dbNot, &DatabaseNotifier::tableChanged, [this](QString tn) {
        if(tn == "expenses") {
            refreshModel();
        }
    });
    refreshModel();
}

ExpensesDockWidgetContents::~ExpensesDockWidgetContents(){
    delete ui;
}

QString ExpensesDockWidgetContents::limitQuery() const {
    auto ofs = (currentPage - 1) * maxRowCount;
    return QString("LIMIT %1 OFFSET %2").arg(maxRowCount).arg(ofs);
}

QString ExpensesDockWidgetContents::whereQuery() const {
    QStringList where;
    auto cbix = ui->comboBulan->currentIndex();
    auto thtx = ui->comboTahun->currentText();
    auto ftst = ui->lineEdit->text().trimmed();
    if(!ftst.isEmpty()) {
        where << QString("division_name || ' ' || description || ' ' || category LIKE '%%1%'").arg(ftst);
    }
    if(cbix > -1) {
        where << QString("strftime('%m', datetime(expense_date)) = '%1'").arg(
            modelBulanan->index(cbix, 0).data(Qt::EditRole).toInt(), 2, 10, QChar('0'));
    }
    where << QString("strftime('%Y', datetime(expense_date)) = '%1'").arg(thtx);
    return where.join(" AND ");
}

void ExpensesDockWidgetContents::refreshModel() {
    int maxPage = qCeil(totalPageCount / (qreal) maxRowCount),
        bounded;
    bounded = qBound(1, ui->spinBox->value(), maxPage);

    ui->spinBox->blockSignals(true);
    ui->spinBox->setMaximum(maxPage);
    currentPage = bounded;
    ui->spinBox->setValue(currentPage);
    ui->spinBox->setSuffix(QString(" / %1").arg(maxPage));
    ui->spinBox->blockSignals(false);
    
    auto qq = QString("%1 WHERE %2 %3").arg(baseQuery, whereQuery(), limitQuery());

    auto onFirstPage = currentPage == 1;
    auto onLastPage = currentPage == maxPage;
    
    ui->firstButton->setDisabled(onFirstPage);
    ui->prevButton->setDisabled(onFirstPage);
    ui->nextButton->setDisabled(onLastPage);
    ui->lastButton->setDisabled(onLastPage);
    
    ui->navPaging->setVisible(maxPage > 1);
    
    qobject_cast<QSqlQueryModel*>(modelExpenses)->setQuery(qq);
}

void ExpensesDockWidgetContents::on_spinBox_valueChanged(int v) {
    currentPage = v;
    refreshModel();
}

void ExpensesDockWidgetContents::on_comboBulan_currentIndexChanged(int ix) {
    auto cq = QString("SELECT COUNT(*) FROM expenses WHERE %1").arg(whereQuery());
    QSqlQuery q(cq);
    q.next();
    totalPageCount = q.value(0).toInt();
    refreshModel();
}

void ExpensesDockWidgetContents::on_comboTahun_currentIndexChanged(const QString& tx) {
    auto cq = QString("SELECT COUNT(*) FROM expenses WHERE %1").arg(whereQuery());
    QSqlQuery q(cq);
    q.next();
    totalPageCount = q.value(0).toInt();
    refreshModel();
}

void ExpensesDockWidgetContents::horizontalHeaderContextMenu(const QPoint& pos){
    auto menu = new QMenu();
    QAction* act;
    auto hhead = ui->tableView->horizontalHeader();
    auto model = ui->tableView->model();
    for(int i=0; i < ui->tableView->horizontalHeader()->count(); ++i) {
        act = menu->addAction(model->headerData(i, Qt::Horizontal).toString());
        act->setProperty("underlyingColumn", i);
        act->setCheckable(true);
        act->setChecked(!hhead->isSectionHidden(i));
    }
    act = menu->exec(hhead->viewport()->mapToGlobal(pos));
    if(act)
        hhead->setSectionHidden(act->property("underlyingColumn").toInt(), !act->isChecked());
    delete menu;
}

QVariant ExpensesProxyModel::data(const QModelIndex& mi, int role) const {
    if(!mi.isValid()) return QVariant();
    auto def = QSortFilterProxyModel::data(mi, role);
    auto columnName = headerData(mi.column(), Qt::Horizontal);
    auto loc = QLocale();
    if(role == Qt::DisplayRole) {
        if(columnName == "Tanggal" || columnName == "Dibuat" || columnName == "Diubah") {
            return loc.toString(def.toDateTime().toLocalTime(), "dddd, d MMM yyyy");
        } else if(columnName == "Jumlah") {
            return loc.toString(def.toLongLong());
        }
    } else if(role == Qt::TextAlignmentRole) {
        if(columnName == "Jumlah") {
            return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
        } else if(columnName != "Deskripsi") {
            return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
        }
    }
    return def;
}

MonthModel::MonthModel(QObject *parent)
: QSqlQueryModel(parent) {
    setQuery("SELECT strftime('%m', expense_date), expense_date FROM expenses GROUP BY strftime('%m', expense_date)");
}

QVariant MonthModel::data(const QModelIndex& mi, int role) const {
    QLocale loc;
    if(role == Qt::DisplayRole) {
        return loc.toString(QSqlQueryModel::data(mi, role).toDateTime().toLocalTime(), "MMMM");
    }
    return QSqlQueryModel::data(mi, role);
}
