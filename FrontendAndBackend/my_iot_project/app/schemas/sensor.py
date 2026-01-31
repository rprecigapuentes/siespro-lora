from pydantic import BaseModel, Field
from datetime import datetime

class SensorInput(BaseModel):
    temperatura: float = Field(..., example=21.0)
    humedad_relativa: float = Field(..., example=54.7)
    # humedad_suelo: ELIMINADO
    rssi: int = Field(..., example=-47)
    snr: int = Field(..., example=9)

class SensorResponse(BaseModel):
    status: str
    bracelet_id: str
    prediction: int
    saved_at: datetime

    class Config:
        from_attributes = True

class DeviceStatus(BaseModel):
    bracelet_id: str
    is_online: bool
    last_seen: datetime
    current_prediction: int
    status_label: str

    class Config:
        from_attributes = True