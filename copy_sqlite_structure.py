import sqlite3
import sys
import os

def get_table_structure(cursor, table_name):
    """Get CREATE TABLE statement for a specific table"""
    cursor.execute(f"SELECT sql FROM sqlite_master WHERE type='table' AND name='{table_name}'")
    return cursor.fetchone()[0]

def get_view_structure(cursor, view_name):
    """Get CREATE VIEW statement for a specific view"""
    cursor.execute(f"SELECT sql FROM sqlite_master WHERE type='view' AND name='{view_name}'")
    return cursor.fetchone()[0]

def get_index_structure(cursor, index_name):
    """Get CREATE INDEX statement for a specific index"""
    cursor.execute(f"SELECT sql FROM sqlite_master WHERE type='index' AND name='{index_name}'")
    return cursor.fetchone()[0]

def get_trigger_structure(cursor, trigger_name):
    """Get CREATE TRIGGER statement for a specific trigger"""
    cursor.execute(f"SELECT sql FROM sqlite_master WHERE type='trigger' AND name='{trigger_name}'")
    return cursor.fetchone()[0]

def copy_database_structure(input_db, output_db):
    """Copy database structure without data"""
    try:
        # Connect to input database
        input_conn = sqlite3.connect(input_db)
        input_cursor = input_conn.cursor()

        # Connect to output database (create if not exists)
        output_conn = sqlite3.connect(output_db)
        output_cursor = output_conn.cursor()

        # Get all tables
        input_cursor.execute("SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%'")
        tables = [row[0] for row in input_cursor.fetchall()]

        # Get all views
        input_cursor.execute("SELECT name FROM sqlite_master WHERE type='view'")
        views = [row[0] for row in input_cursor.fetchall()]

        # Get all indexes
        input_cursor.execute("SELECT name FROM sqlite_master WHERE type='index' AND name NOT LIKE 'sqlite_%'")
        indexes = [row[0] for row in input_cursor.fetchall()]

        # Get all triggers
        input_cursor.execute("SELECT name FROM sqlite_master WHERE type='trigger'")
        triggers = [row[0] for row in input_cursor.fetchall()]

        # Copy tables
        for table in tables:
            create_table_sql = get_table_structure(input_cursor, table)
            output_cursor.execute(create_table_sql)

        # Copy views
        for view in views:
            create_view_sql = get_view_structure(input_cursor, view)
            output_cursor.execute(create_view_sql)

        # Copy indexes
        for index in indexes:
            create_index_sql = get_index_structure(input_cursor, index)
            output_cursor.execute(create_index_sql)

        # Copy triggers
        for trigger in triggers:
            create_trigger_sql = get_trigger_structure(input_cursor, trigger)
            output_cursor.execute(create_trigger_sql)

        # Commit changes and close connections
        output_conn.commit()
        input_conn.close()
        output_conn.close()
        print(f"Database structure copied successfully to {output_db}")

    except sqlite3.Error as e:
        print(f"Error: {e}")
        sys.exit(1)

def main():
    if len(sys.argv) != 2:
        print("Usage: python copy_sqlite_structure.py <input_database_file>")
        sys.exit(1)

    input_db = sys.argv[1]
    if not os.path.exists(input_db):
        print(f"Error: Database file {input_db} does not exist")
        sys.exit(1)

    output_db = f"copy_{os.path.basename(input_db)}"
    copy_database_structure(input_db, output_db)

if __name__ == "__main__":
    main()