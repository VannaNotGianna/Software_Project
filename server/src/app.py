from datetime import datetime, timedelta

import uvicorn
import functools
from fastapi import FastAPI, Depends
from fastapi.staticfiles import StaticFiles
from sqlalchemy.orm import Session

import schemas, database
from database import SessionLocal

import data

database.Base.metadata.create_all(bind=database.engine)

app = FastAPI()
app.mount("/analytics", StaticFiles(directory="site", html=True), name="site")

def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()

@app.get("/hit")
def hit_database(db: Session = Depends(get_db)):
    return data.create_hit(db)

@app.get("/last")
def last_queries(period: int, db: Session = Depends(get_db)):
    dt = datetime.now() - timedelta(seconds=period)
    return data.get_recent_hits(db, dt)

if __name__ == '__main__':
    uvicorn.run(app, host="127.0.0.1", port=8080)
