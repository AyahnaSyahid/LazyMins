#include "querycombobox.h"

#include <QSqlQueryModel>
#include <QSqlQuery>
#include <QSqlError>
#include <QTableView>
#include <QHeaderView>
#include <QTimer>
#include <QMessageBox>

QueryComboBox::QueryComboBox(QWidget *p):
qmodel(new QSqlQueryModel(this)), boxView(new QTableView), QComboBox(p)
{
  setModel(qmodel);
  setModelColumn(1);
  
  setView(boxView);
  
  auto vh = boxView->verticalHeader();
  vh->setMinimumSectionSize(22);
  vh->setDefaultSectionSize(20);
  vh->hide();
  boxView->setHorizontalScrollMode(QTableView::ScrollPerPixel);
  boxView->setSelectionBehavior(QTableView::SelectRows);
  boxView->horizontalHeader()->hide();
  boxView->setAlternatingRowColors(true);
  boxView->setMaximumHeight(20 * 10);
}

void QueryComboBox::setQuery(const QString& s, QSqlDatabase &db){
  m_db = db;
  m_bindings.clear();
  m_query = s;
  qmodel->setQuery(s, m_db);
}

void QueryComboBox::setQuery(const QString& s, const QVariantMap &binding, QSqlDatabase &db){
    m_db = db;
    QSqlQuery q(m_db);
    q.prepare(s);
    for(auto [k, v] : binding.asKeyValueRange()) {
        q.bindValue(k, v);
    }

    if(!q.exec()) {
        m_query.clear();
        m_bindings.clear();
        m_db = QSqlDatabase::database();

        // Capture 'text' by value agar aman saat lambda dieksekusi nanti
        QString text = q.lastError().isValid()
                           ? q.lastError().text()
                           : "Error bukan dari query";

        QTimer::singleShot(0, this, [text, this]() {
            popError(text);
        });
        return;
    }

    m_query = s;
    m_bindings = binding;
    m_db = db;
    qmodel->setQuery(std::move(q));
}

void QueryComboBox::refetchData() {
    int cix = currentIndex();

    if (!m_bindings.isEmpty()) {
        setQuery(m_query, m_bindings, m_db);
    } else {
        setQuery(m_query, m_db); // ✅ pakai m_query, bukan queryText
    }

    setCurrentIndex(cix >= qmodel->rowCount() ? qmodel->rowCount() - 1 : cix);
}

void QueryComboBox::popError(const QString& s) const
{
  QMessageBox::warning(nullptr, "Error", s);
}

void QueryComboBox::boxViewAutoResize() {
  boxView->resizeColumnsToContents();
  boxView->setMinimumWidth(boxView->horizontalHeader()->length() + 17); // 17 untuk scrollbar dan padding
}

int QueryComboBox::findValue(const QVariant& value, int column) const{
    for(int i = 0; i < qmodel->rowCount(); i++) {
        if (qmodel->index(i, column).data() == value) {
            return i;
        }
    }
    return -1;
}

void QueryComboBox::showColumn(int column, bool show)
{
    boxView->setColumnHidden(column, !show);
}
