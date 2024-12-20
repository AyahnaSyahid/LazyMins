# test date QSQLITE
"""
Kesimpulan:
    QDateTime() menghasilkan waktu local time
    CURRENT_TIMESTAMP menghasilkan waktu UTC
    datetime('now', 'localtime') menghasilkan local time

Pelajaran:
    Selalu konversi ke Utc sebelum menyimpan ke database
    Selalu konversi ke Local jika ingin di lihat oleh pengguna

"""

from PyQt5.QtSql import QSqlDatabase, QSqlTableModel, QSqlQuery
from PyQt5.QtCore import QDateTime
from PyQt5.QtWidgets import QApplication, QTableView

if __name__ == "__main__":
    base = QSqlDatabase.addDatabase("QSQLITE")
    base.setDatabaseName(":memory:")
    base.open()
    q = QSqlQuery()
    q.exec("CREATE TABLE a ( curdate DATETIME, "
                "curdateUtc DATETIME, internal_ts_dt DATETIME, "
                "internal_ts TIMESTAMP, internal_local_f);")
    q.prepare("INSERT INTO a VALUES (?, ?, CURRENT_TIMESTAMP, CURRENT_TIMESTAMP, datetime('now', 'localtime'));")
    q.addBindValue(QDateTime.currentDateTime())
    q.addBindValue(QDateTime.currentDateTimeUtc())
    if not q.exec():
        print(q.lastError().text())
        exit
    # second row all_local_converted ts
    q.exec("SELECT * FROM a WHERE ROWID = 1")
    res = []
    if q.next():
        res.append(q.value(0))
        res.append(q.value(1))
        res.append(q.value(2))
        res.append(q.value(3))
        res.append(q.value(4))
    for i in res:
        print(i)
    exit()
    
    app = QApplication([])
    view = QTableView()
    model = QSqlTableModel()
    model.setTable('a')
    model.select()
    view.setModel(model)
    view.show()
    app.exec()
    