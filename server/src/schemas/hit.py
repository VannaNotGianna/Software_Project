from sqlalchemy import Boolean, Column, ForeignKey, Integer, String, DateTime, sql, TIMESTAMP
from sqlalchemy.orm import relationship
from time import time

from database import Base


class Hit(Base):
    __tablename__ = "hits"

    id = Column(Integer, primary_key=True)
    timestamp = Column(Integer(), nullable=False, default=lambda: int(time()))


