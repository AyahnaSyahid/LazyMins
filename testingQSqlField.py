from PySide6.QtCore import QCoreApplication
from PySide6.QtSql import QSqlDatabase, QSqlRecord, QSqlField

if __name__ == '__main__':
    app = QCoreApplication([])
    db = QSqlDatabase.addDatabase('QSQLITE')
    db.setDatabaseName("LAdmins.db")
    if not db.open():
        print("Gagal membuka database")
        exit(1)
    rc = db.record("orders")
    for i in range(rc.count()):
        field = rc.field(i)
        print(f'{field.name():<19} | {field.metaType().name():<8} | {field.isAutoValue():<5} | {field.requiredStatus() == field.RequiredStatus.Required}')
