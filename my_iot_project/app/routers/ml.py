from fastapi import APIRouter, Depends
from app.services.ml_service import ml_service
from app.schemas.sensor import SensorInput
from app.core.security import get_current_user

router = APIRouter(prefix="/ml", tags=["Machine Learning"])

# ❌ ANTES: @POST(...)
# ✅ AHORA: @router.post(...)
@router.post("/predict")
def predict_only(data: SensorInput, _ = Depends(get_current_user)):
    """Solo predice, no guarda en BD"""
    # Nota: Si usas Pydantic v2, usa data.model_dump(). Si es v1, data.dict() funciona bien.
    pred = ml_service.predict(data.dict())
    return {"prediction": pred}

@router.post("/reload-model")
def reload_model(_ = Depends(get_current_user)):
    """Recarga los archivos .pkl sin reiniciar el servidor"""
    ml_service.load_models()
    return {"status": "Modelos recargados correctamente"}