from sqlite3 import connect
import sqlite3
import random
import string 

allowed_chars = string.ascii_letters + ' ' + string.digits

def genRand(alist, length):
    text = random.choice(alist)
    for i in range(1, length):
        text += random.choice(alist)
    return text

def gen__(length):
    global allowed_chars
    return genRand(allowed_chars, length)

def generate_rdata(src, tgt):
    prices = [i * 500 for i in range(1, 149)]
    with connect(src) as s:
        with connect(tgt) as t:
            # Customer
            cur = s.execute("SELECT klien FROM a3pdata group by klien Having count(klien) > 40")
            for c in cur:
                t.execute('INSERT INTO customers (name, address, phone) VALUES (?, ?, ?)', (c[0], gen__(31), genRand(string.digits, 12)))
            print("customers generated")
            # Product
            cur = s.execute("SELECT bahan FROM a3pdata GROUP BY bahan HAVING count(bahan) > 20")
            for c in cur:
                cst = random.choice(prices);
                t.execute("INSERT INTO products (name, description, use_area, cost, base_price, material, category) VALUES (?, ?, ?, ?, ?, ?, ?)",
                            (c[0], f'Print A3P bahan {c[0]}', 0, cst, cst + random.choice([i * 100 for i in range(1, 51)]), f'Kertas {c[0]}', 'Print'))
            print("products generated")
            
            # Orders
            cur = s.execute('SELECT date(created) AS cDate, klien, bahan, file, jkertas * jkopi as qty from a3pdata')
            t.execute('CREATE TEMP TABLE a3ord (cDate TEXT, klien TEXT, bahan TEXT, file TEXT, qty INTEGER)')
            for c in cur:
                t.execute("INSERT INTO a3ord VALUES (?, ?, ?, ?, ?)", c)
            try:
                t.execute("INSERT INTO orders (order_date, user_id, customer_id, product_id, name, use_area, width, height, quantity, production_cost, unit_price, discount, status) "
                           "SELECT tmp.cDate, 1, cs.customer_id, pr.product_id, tmp.file, pr.use_area, 1, 1, qty, pr.cost, pr.base_price, 0, 'OK' "
                           "FROM a3ord tmp "
                           "JOIN products pr ON pr.name = tmp.bahan "
                           "JOIN customers cs ON cs.name = tmp.klien")
            except sqlite3.Error as sqlerr:
                print(sqlerr)
                return
            print("orders generated")
            t.execute("DROP TABLE a3ord")

def main():
    source = 'a3p.db'
    target = 'target.db'
    generate_rdata(source, target)
    
    

if __name__ == '__main__':
    main()