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
def receive_sensor_data(data: SensorInput, db: Session = Depends(get_db)):
    try:
        # 1. Predicción (El servicio ML también debe actualizarse, ver abajo)
        try:
            prediction_result = ml_service.predict(data.dict())
        except Exception as e:
            print(f"❌ Error en ML: {e}")
            prediction_result = -1 

        # 2. Crear objeto (SIN humedad_suelo)
        db_sensor = SensorData(
            bracelet_id=DEMO_ID,
            temperatura=data.temperatura,
            humedad_relativa=data.humedad_relativa,
            # humedad_suelo=data.humedad_suelo, <-- ELIMINADO
            rssi=data.rssi,
            snr=data.snr,
            prediction=prediction_result
        )
        
        # 3. Guardar en DB
        db.add(db_sensor)
        db.commit()
        db.refresh(db_sensor)
        
        return {
            "status": "ok",
            "bracelet_id": db_sensor.bracelet_id,
            "prediction": prediction_result,
            "saved_at": db_sensor.created_at
        }

    except Exception as e:
        # ... (manejo de error igual que antes) ...
        db.rollback()
        import traceback
        return {"status": "error", "detail": str(e)}
    
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