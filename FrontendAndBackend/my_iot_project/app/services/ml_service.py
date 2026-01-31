import joblib
import pandas as pd
import os

class MLService:
    def __init__(self):
        self.model = None
        self.preprocessor = None
        self.model_path = "ml/rf_model.pkl"
        self.preprocessor_path = "ml/preprocessor.pkl"
        self.load_models()

    def load_models(self):
        """Carga o recarga los modelos desde disco"""
        try:
            if os.path.exists(self.model_path):
                self.model = joblib.load(self.model_path)
                print(f"✅ Modelo cargado desde {self.model_path}")
            else:
                print(f"⚠️ No se encontró {self.model_path}")

            if os.path.exists(self.preprocessor_path):
                self.preprocessor = joblib.load(self.preprocessor_path)
                print(f"✅ Preprocesador cargado desde {self.preprocessor_path}")
            else:
                print(f"⚠️ No se encontró {self.preprocessor_path}")
                
        except Exception as e:
            print(f"❌ Error cargando modelos: {e}")

    def predict(self, data: dict) -> int:
        if not self.model:
            return -1
        
        try:
            df = pd.DataFrame([data])
            
            # ⚠️ AQUÍ ESTÁ EL ERROR: Seguramente todavía tienes 'humedad_suelo' aquí.
            # ✅ CORRECCIÓN: Deja SOLO estas 4 variables:
            features = ['temperatura', 'humedad_relativa', 'rssi', 'snr']
            
            # Seleccionar solo las columnas que existen
            X = df[features]

            if self.preprocessor:
                X = self.preprocessor.transform(X)

            prediction = self.model.predict(X)
            return int(prediction[0])
        except Exception as e:
            # Este es el print que estás viendo en tu consola
            print(f"Error en predicción: {e}")
            return -1

# Instancia global
ml_service = MLService()