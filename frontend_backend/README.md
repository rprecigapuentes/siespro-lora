# 🛡️ SIESPRO: AI-Powered IoT Guardian

![Project Status](https://img.shields.io/badge/Status-Live_Demo-success) ![Tech Stack](https://img.shields.io/badge/Stack-Python_%7C_Tailwind_%7C_Scikit--Learn-00D9FF) ![AI Model](https://img.shields.io/badge/AI-Random_Forest-orange)

> **"Control Total en Tiempo Real"** - A next-generation safety monitoring system that combines low-level IoT telemetry with high-level Machine Learning predictions.

## 💡 The Project
**SIESPRO** is an intelligent dashboard designed to monitor the safety status of wearable IoT devices in real-time. Unlike traditional dashboards that only show raw data, SIESPRO uses a **Machine Learning model** to interpret signal quality and environmental data, predicting whether a user is in a "Safe Zone" or at "Risk" (e.g., wandering off-limits or disconnecting).

### 🔗 [View the Live Demo on Render](https://siespro.onrender.com)
*(Note: If the server is sleeping, please allow 30 seconds for the backend to wake up)*

---

## 📸 Interface Preview
![Dashboard Preview](https://via.placeholder.com/800x400?text=Insert+Your+Dashboard+Screenshot+Here)

## 🚀 Key Features

### 🧠 The Intelligence (Backend & AI)
We moved beyond simple threshold alerts. The system utilizes a **Random Forest Classifier** trained on historical telemetry to make decisions.
* **Model:** Scikit-Learn Random Forest (`n_estimators=100`).
* **Input Features:** * `RSSI` & `SNR`: Signal strength and noise ratios are used as proxies for geolocation/distance.
    * `Temperature` & `Humidity`: Environmental context.
* **Preprocessing:** `StandardScaler` is used to normalize signal dBm (negative values) against environmental data (positive values) for accurate inference.

### 🎨 The Experience (Frontend)
* **Cyberpunk UI:** Built with **TailwindCSS** and raw HTML5 for maximum performance (no heavy framework overhead).
* **Visual Feedback:** * Neon glow effects indicate system status.
    * Dynamic mapping simulation based on device ID hashing.
* **Resiliency:** The UI handles 3 states automatically:
    1.  🟢 **Safe:** AI predicts user is within bounds.
    2.  🔴 **Danger:** AI predicts user is out of bounds/risk.
    3.  ⚪ **Offline:** Device telemetry stopped (Logic handles connection drops).

---

## 🛠️ Tech Stack & Architecture

| Component | Technology | Description |
| :--- | :--- | :--- |
| **Frontend** | HTML5 / JS / TailwindCSS | A single-page application (SPA) that polls the API every 3s. |
| **Backend** | Python (FastAPI/Flask) | Handles data ingestion and serves predictions. |
| **ML Engine** | Scikit-Learn | Trains `rf_model.pkl` and scales data via `preprocessor.pkl`. |
| **Hosting** | Render | Cloud deployment for the API. |

## 💻 Installation & Local Testing

If you want to run the training engine locally:

1.  **Clone the Repository:**
    ```bash
    git clone [https://github.com/ssiespro-web/Siespro.git](https://github.com/ssiespro-web/Siespro.git)
    cd ssiespro-web/Siespro
    ```

2.  **Train the Model:**
    Navigate to the project folder and install dependencies.
    ```bash
    cd my_iot_project
    pip install -r requirements.txt
    
    # Run the training script to generate new .pkl files
    python train_real_model.py
    ```
    *Output:* This will generate `ml/rf_model.pkl` with an accuracy report (Confusion Matrix).

3.  **Run the Dashboard:**
    Simply open `index.html` in any modern web browser. 
    *Config:* By default, it connects to the production API. To test locally, change `const API_URL` in line 305 to `http://localhost:8000`.

## 📊 Data Science Approach

We approached the classification problem by analyzing the correlation between signal degradation (RSSI/SNR) and environmental factors.

```python
# Snippet from train_real_model.py
features = ['temperatura', 'humedad_relativa', 'rssi', 'snr']
rf = RandomForestClassifier(n_estimators=100, random_state=42)
rf.fit(X_train_scaled, y_train)
