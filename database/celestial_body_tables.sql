CREATE TABLE IF NOT EXISTS TargetBody (
   bodyId INTEGER PRIMARY KEY,
   bodyName TEXT UNIQUE NOT NULL  
);

CREATE TABLE IF NOT EXISTS Ephemeris (
   bodyId INTEGER,
   time TEXT NOT NULL,
   azimuth REAL NOT NULL,
   elevation REAL NOT NULL,
   FOREIGN KEY(bodyId) REFERENCES TargetBody(bodyId)
);