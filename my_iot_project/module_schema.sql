-- Habilitar extensión para UUIDs (estándar en Supabase)
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- 1. Tabla Users
CREATE TABLE IF NOT EXISTS users (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    username VARCHAR(50) UNIQUE NOT NULL,
    api_key VARCHAR(100) UNIQUE NOT NULL,
    is_active BOOLEAN DEFAULT TRUE,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT timezone('utc', now())
);

-- 2. Tabla Sensor Data
CREATE TABLE IF NOT EXISTS sensor_data (
    id SERIAL PRIMARY KEY,
    bracelet_id VARCHAR(50) NOT NULL,
    temperatura FLOAT NOT NULL,
    humedad_relativa FLOAT NOT NULL,
    humedad_suelo FLOAT NOT NULL,
    rssi INT NOT NULL,
    snr INT NOT NULL,
    prediction INT, -- Puede ser nulo si falla el ML, o 0/1
    created_at TIMESTAMP WITH TIME ZONE DEFAULT timezone('utc', now())
);

-- Indices para mejorar velocidad de consulta por manilla
CREATE INDEX idx_sensor_bracelet ON sensor_data(bracelet_id);
CREATE INDEX idx_users_api_key ON users(api_key);