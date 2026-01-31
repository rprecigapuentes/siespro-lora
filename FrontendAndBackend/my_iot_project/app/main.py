from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from app.core.config import settings
from app.routers import sensors, ml, auth

app = FastAPI(
    title=settings.PROJECT_NAME,
    version=settings.VERSION
)

# Configuración CORS (Ajustar allow_origins en prod)ls
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Registrar Routers
app.include_router(auth.router)
app.include_router(sensors.router)
app.include_router(ml.router)

@app.get("/")
def root():
    return {"message": "IoT ML API is running", "docs": "/docs"}