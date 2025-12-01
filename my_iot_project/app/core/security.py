from fastapi import Security, HTTPException, status, Depends
from fastapi.security.api_key import APIKeyHeader
from sqlalchemy.orm import Session
from app.core.database import get_db
from app.core.config import settings
from app.models.user import User

api_key_header = APIKeyHeader(name=settings.API_KEY_NAME, auto_error=False)

async def get_current_user(
    api_key: str = Security(api_key_header), 
    db: Session = Depends(get_db)
):
    if not api_key:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Falta la API Key"
        )
    
    user = db.query(User).filter(User.api_key == api_key).first()
    
    if not user:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="API Key inválida"
        )
    
    return user