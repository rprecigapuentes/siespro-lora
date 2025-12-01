from pydantic import BaseModel
from uuid import UUID

class UserCreate(BaseModel):
    username: str

class UserResponse(BaseModel):
    id: UUID
    username: str
    api_key: str

    class Config:
        from_attributes = True