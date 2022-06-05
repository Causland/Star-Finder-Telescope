import sqlite3
import pandas as pd

# Create a sqlite3 database for the telescope to use for position queries
conn = sqlite3.connect('celestial_body_data.db')
c = conn.cursor()

# Create a table for the HYG-Database dataset
c.execute("CREATE TABLE IF NOT EXISTS hygdata ("
   "id INTEGER PRIMARY KEY,"
   "hip INTEGER,"
   "hd INTEGER,"
   "hr INTEGER,"
   "gl INTEGER,"
   "bf TEXT,"
   "proper TEXT,"
   "ra REAL,"
   "dec REAL,"
   "dist REAL,"
   "pmra REAL,"
   "pmdec REAL,"
   "rv REAL,"
   "mag REAL,"
   "absmag REAL,"
   "spect TEXT,"
   "ci REAL,"
   "x REAL,"
   "y REAL,"
   "z REAL,"
   "vx REAL,"
   "vy REAL,"
   "vz REAL,"
   "rarad REAL,"
   "decrad REAL,"
   "pmrarad REAL,"
   "pmdecrad REAL,"
   "bayer INTEGER,"
   "flam INTEGER,"
   "con TEXT,"
   "comp TEXT,"
   "comp_primary TEXT,"
   "base TEXT,"
   "lum REAL,"
   "var TEXT,"
   "var_min REAL,"
   "var_max REAL"
   ")")

# Populate the hygdata table with data from the hygdata_v3.csv file
data = pd.read_csv('hygdata_v3.csv')
data.to_sql('hygdata', conn, if_exists='append', index=False)

