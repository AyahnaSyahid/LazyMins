import sqlite3

def get_list_table(db):
    con = sqlite3.connect(db)
    cur = con.execute("SELECT tbl_name FROM sqlite_master")
    while True:
        a = cur.fetchone()
        if a is None:
            raise StopIteration
            break
        yield a

def get_table_ddl(db, table_name):
    con = sqlite3.connect(db)
    cur = con.execute("SELECT sql FROM sqlite_master WHERE tbl_name = ? and type = 'table'", (table_name,))
    return cur.fetchone()

def emptied_database(db):
    con = sqlite3.connect(db)
    tc = []
    for result in con.execute("SELECT sql FROM sqlite_master WHERE type = ?", ("table",)):
        tc.append(result[0])
    
    if len(tc):
        con2 = sqlite3.connect(db + "_emptied.db")
        for i in tc:
            con2.execute(i.replace("  ", " "))


if __name__ == "__main__":
    emptied_database("sample_src.db")
    # w = get_table_ddl("sample_src.db", "orders")
    # print(w)