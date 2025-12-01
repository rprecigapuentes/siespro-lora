import pandas as pd
import joblib
from sklearn.ensemble import RandomForestClassifier
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import train_test_split
from sklearn.metrics import classification_report, confusion_matrix

# 1. Cargar datos
print("🔄 Cargando dataset.csv...")
df = pd.read_csv('dataset.csv')

# 2. Renombrar columnas para que coincidan con la API (app/schemas/sensor.py)
# Tus datos vs. Lo que espera el sistema
column_mapping = {
    'temp_C': 'temperatura',
    'hum_aire_pct': 'humedad_relativa',
    'hum_tierra_pct': 'humedad_suelo',
    'rssi_dBm': 'rssi',
    'snr_dB': 'snr',
    'label': 'target'
}
df = df.rename(columns=column_mapping)

# 3. Seleccionar features en el ORDEN EXACTO que usa ml_service.py
features = ['temperatura', 'humedad_relativa', 'humedad_suelo', 'rssi', 'snr']
X = df[features]
y = df['target']

# 4. Split de prueba (solo para verificar calidad)
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# 5. Preprocesamiento (StandardScaler)
scaler = StandardScaler()
X_train_scaled = scaler.fit_transform(X_train)
X_test_scaled = scaler.transform(X_test)

# 6. Entrenar Modelo
print("🧠 Entrenando Random Forest...")
rf = RandomForestClassifier(n_estimators=100, random_state=42)
rf.fit(X_train_scaled, y_train)

# 7. Evaluar
y_pred = rf.predict(X_test_scaled)
print("\n📊 Resultados del Modelo:")
print(confusion_matrix(y_test, y_pred))
print(classification_report(y_test, y_pred))

# 8. Re-entrenar con TODOS los datos para producción
# (Mejor práctica: usar toda la data disponible para el modelo final)
full_scaler = StandardScaler()
X_scaled_full = full_scaler.fit_transform(X)
final_model = RandomForestClassifier(n_estimators=100, random_state=42)
final_model.fit(X_scaled_full, y)

# 9. Guardar artefactos
joblib.dump(final_model, 'ml/rf_model.pkl')
joblib.dump(full_scaler, 'ml/preprocessor.pkl')

print("✅ Modelo real guardado en ml/rf_model.pkl")
print("✅ Preprocesador guardado en ml/preprocessor.pkl")
print("🚀 ¡Reinicia tu servidor FastAPI para cargar el nuevo modelo!")