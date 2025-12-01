from sqlalchemy import Column, Integer, String, Float, DateTime, func
from app.core.database import Base

class SensorData(Base):
    __tablename__ = "sensor_data"

    id = Column(Integer, primary_key=True, index=True)
    bracelet_id = Column(String, index=True)
    temperatura = Column(Float)
    humedad_relativa = Column(Float)
    humedad_suelo = Column(Float)
    rssi = Column(Integer)
    snr = Column(Integer)
    prediction = Column(Integer)
    created_at = Column(DateTime(timezone=True), server_default=func.now())