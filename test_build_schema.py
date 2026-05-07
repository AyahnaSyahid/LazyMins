import sqlite3
import sys
import os
from pathlib import Path

def initialize_schema_file(file_name: str, db_path: str) -> bool:
    if not os.path.exists(file_name):
        print(f"[ERROR] Schema file tidak ditemukan: {file_name}")
        return False

    statements = []
    statement = ""
    inside_create_trigger = False
    case_depth = 0

    with open(file_name, "r", encoding="utf-8") as f:
        for line in f:
            line = line.rstrip("\n").rstrip("\r")

            if not line.strip():
                continue
            if line.strip().startswith("--"):
                continue

            if "create trigger" in line.lower():
                inside_create_trigger = True
                case_depth = 0
                statement += "\n" + line
                continue

            if inside_create_trigger:
                stripped = line.strip().lower()

                # Lacak kedalaman CASE
                if stripped.startswith("case"):
                    case_depth += 1

                statement += "\n" + line

                if "end;" in stripped:
                    if case_depth > 0:
                        case_depth -= 1  # END; milik CASE
                    else:
                        inside_create_trigger = False  # END; milik trigger
                        statements.append(statement.strip())
                        statement = ""
                continue

            if ";" in line:
                statement += "\n" + line
                statements.append(statement.strip())
                statement = ""
                continue

            statement += "\n" + line

    if statement.strip():
        statements.append(statement.strip())

    print(f"[INFO] Menggunakan database: {db_path}")
    print(f"[INFO] Total statements: {len(statements)}")

    try:
        con = sqlite3.connect(db_path)
        cur = con.cursor()
        con.execute("BEGIN")

        for i, st in enumerate(statements):
            try:
                cur.execute(st)
                print(f"[OK] Statement {i+1}: {st[:60].strip()}")
            except sqlite3.Error as e:
                print(f"[ERROR] Statement {i+1} gagal: {e}")
                print(f"        SQL: {st}")
                con.rollback()
                con.close()
                return False
        
        # insert root
        con.execute("""
        INSERT INTO admins VALUES 
            ( 1, 1, 'root', 
              'f1fd6123ff31d314269652592839d7a90040c3409ff4be05546fe512156454b0', 
              '7v3wnEFUTiNfh2wlKQlUAyNF2WV9Uudg', '-', 'Aksarajata@AJ.com', '-',
              1, '2026-04-09T13:22:28.000Z', '2026-04-09T11:00:20.678Z', 
              '2026-04-09T11:00:20.678Z' )""")
        
        con.commit()
        con.close()
        print("[INFO] Semua statement berhasil.")
        return True

    except sqlite3.Error as e:
        print(f"[ERROR] Koneksi database gagal: {e}")
        return False


if __name__ == "__main__":
    arg_len = len(sys.argv)
    
    if arg_len == 3:
        success = initialize_schema_file(sys.argv[1], sys.argv[2])
        sys.exit(0 if success else 1)
    elif arg_len == 1:
        file_dir = Path(__file__).parent
        success = initialize_schema_file(file_dir / "resources/schema/percetakan_schema_improved.sql", file_dir / "LAdmins.db")
        sys.exit(0 if success else 1)
    else:
        print("Usage: python script.py <schema_file.sql> <database.db>")
        sys.exit(1)
