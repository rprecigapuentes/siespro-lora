import pandas as pd
import joblib
from sklearn.ensemble import RandomForestClassifier
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import train_test_split
from sklearn.metrics import classification_report, confusion_matrix

print("🔄 Cargando dataset.csv...")
df = pd.read_csv('dataset.csv')

# Mapeo (Quitamos humedad_tierra_pct si existía en tu CSV original)
column_mapping = {
    'temp_C': 'temperatura',
    'hum_aire_pct': 'humedad_relativa',
    # 'hum_tierra_pct': 'humedad_suelo', <-- IGNORAMOS ESTA COLUMNA DEL CSV
    'rssi_dBm': 'rssi',
    'snr_dB': 'snr',
    'label': 'target'
}
df = df.rename(columns=column_mapping)

# Definir features (SIN SUELO)
features = ['temperatura', 'humedad_relativa', 'rssi', 'snr']

X = df[features]
y = df['target']

# Split
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# Preprocesamiento
scaler = StandardScaler()
X_train_scaled = scaler.fit_transform(X_train)
X_test_scaled = scaler.transform(X_test)

# Entrenar
print("🧠 Entrenando Random Forest (Sin Humedad Suelo)...")
rf = RandomForestClassifier(n_estimators=100, random_state=42)
rf.fit(X_train_scaled, y_train)

# Evaluar
y_pred = rf.predict(X_test_scaled)
print("\n📊 Resultados:")
print(confusion_matrix(y_test, y_pred))
print(classification_report(y_test, y_pred))

# Modelo Final Completo
full_scaler = StandardScaler()
X_scaled_full = full_scaler.fit_transform(X)
final_model = RandomForestClassifier(n_estimators=100, random_state=42)
final_model.fit(X_scaled_full, y)

# Guardar
joblib.dump(final_model, 'ml/rf_model.pkl')
joblib.dump(full_scaler, 'ml/preprocessor.pkl')

print("✅ Nuevos modelos generados en /ml (4 features)")