from fastapi import APIRouter, Depends
from sqlalchemy.orm import Session
from sqlalchemy import text
from typing import List
from datetime import datetime, timezone

from app.core.database import get_db
from app.schemas.sensor import SensorInput, DeviceStatus
from app.models.sensor import SensorData
from app.services.ml_service import ml_service

router = APIRouter(prefix="/sensors", tags=["Sensors"])

# ID por defecto para la demo
DEMO_ID = "MANILLA-DEMO-01"

@router.post("/data")
def receive_sensor_data(
    data: SensorInput, 
    db: Session = Depends(get_db)
):
    # 1. Realizar predicción
    # (El modelo solo necesita los números, así que data.dict() funciona perfecto)
    prediction_result = ml_service.predict(data.dict())
    
    # 2. Crear objeto para DB
    # AQUÍ PONEMOS EL ID AUTOMÁTICO
    db_sensor = SensorData(
        bracelet_id=DEMO_ID,  # <--- ID Fijo
        temperatura=data.temperatura,
        humedad_relativa=data.humedad_relativa,
        humedad_suelo=data.humedad_suelo,
        rssi=data.rssi,
        snr=data.snr,
        prediction=prediction_result
    )
    
    # 3. Guardar en DB
    db.add(db_sensor)
    db.commit()
    db.refresh(db_sensor)
    
    # 4. Retornar
    return {
        "status": "ok",
        "bracelet_id": db_sensor.bracelet_id,
        "prediction": prediction_result,
        "saved_at": db_sensor.created_at
    }

# --- El endpoint Monitor queda igual, pero ahora solo verá la MANILLA-DEMO-01 ---
OFFLINE_THRESHOLD_SECONDS = 120 

@router.get("/monitor", response_model=List[DeviceStatus])
def monitor_devices(db: Session = Depends(get_db)):
    query = text("""
        SELECT DISTINCT ON (bracelet_id) *
        FROM sensor_data
        ORDER BY bracelet_id, created_at DESC
    """)
    results = db.execute(query).fetchall()
    
    devices_status = []
    now = datetime.now(timezone.utc)

    for row in results:
        time_diff = now - row.created_at
        is_online = time_diff.total_seconds() < OFFLINE_THRESHOLD_SECONDS
        label = "PELIGRO/AFUERA" if row.prediction == 1 else "SEGURO/ADENTRO"

        devices_status.append(
            DeviceStatus(
                bracelet_id=row.bracelet_id,
                is_online=is_online,
                last_seen=row.created_at,
                current_prediction=row.prediction if row.prediction is not None else -1,
                status_label=label
            )
        )
    return devices_status