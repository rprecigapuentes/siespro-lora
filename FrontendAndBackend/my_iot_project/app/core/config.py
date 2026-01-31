from pydantic_settings import BaseSettings

class Settings(BaseSettings):
    PROJECT_NAME: str = "IoT ML API"
    VERSION: str = "1.0.0"
    DATABASE_URL: str
    API_KEY_NAME: str = "x-api-key"

    class Config:
        env_file = ".env"

settings = Settings()