from fastapi import APIRouter, Depends, HTTPException
from sqlalchemy.orm import Session
import secrets
from app.core.database import get_db
from app.models.user import User
from app.schemas.user import UserCreate, UserResponse
from app.core.security import get_current_user

# Creamos la instancia del router
router = APIRouter(prefix="/auth", tags=["Auth"])

# ❌ INCORRECTO: @POST("/create-user")
# ✅ CORRECTO: @router.post("/create-user")
@router.post("/create-user", response_model=UserResponse)
def create_user(user_in: UserCreate, db: Session = Depends(get_db)):
    # Generar API Key segura
    generated_key = secrets.token_urlsafe(32)
    
    new_user = User(
        username=user_in.username,
        api_key=generated_key
    )
    db.add(new_user)
    try:
        db.commit()
        db.refresh(new_user)
    except Exception: # Es buena práctica capturar excepciones genéricas o específicas de integridad
        db.rollback()
        raise HTTPException(status_code=400, detail="El usuario ya existe")
        
    return new_user

# ❌ INCORRECTO: @GET("/me")
# ✅ CORRECTO: @router.get("/me")
@router.get("/me", response_model=UserResponse)
def get_me(current_user: User = Depends(get_current_user)):
    return current_user