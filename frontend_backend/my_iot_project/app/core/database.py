from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker, declarative_base
from app.core.config import settings

# Crear engine
engine = create_engine(
    settings.DATABASE_URL,
    echo=True,  # <--- AGREGA ESTO: Muestra todo lo que hace la DB en consola
    pool_pre_ping=True # <--- AGREGA ESTO: Intenta revivir conexiones muertas
)
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)

Base = declarative_base()

# Dependency para inyección en rutas
def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()