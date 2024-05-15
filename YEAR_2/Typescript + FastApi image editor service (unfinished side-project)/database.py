import sqlite3

conn = sqlite3.connect('db.db')

cursor = conn.cursor()

cursor.execute('''CREATE TABLE IF NOT EXISTS images
                (id INTEGER PRIMARY KEY, title TEXT, width INTEGER, height INTEGER)''')

cursor.execute('''CREATE TABLE IF NOT EXISTS rectangles
                (id INTEGER PRIMARY KEY, image_id INTEGER, x1 INTEGER, y1 INTEGER, x2 INTEGER, y2 INTEGER,
                color TEXT,
                FOREIGN KEY (image_id) REFERENCES images(id))''')

conn.commit()

conn.close()