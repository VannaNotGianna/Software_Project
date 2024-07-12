from datetime import datetime
from sqlalchemy.orm import Session

from schemas.hit import Hit

def create_hit(db: Session) -> Hit:
    hit_item = Hit()
    db.add(hit_item)
    db.commit()
    db.refresh(hit_item)
    return hit_item

def get_recent_hits(db: Session, since: datetime) -> list[Hit]:
    return db.query(Hit).filter(Hit.timestamp >= int(since.timestamp())).all()
