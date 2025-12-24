# System Architecture

## Workflow
1. **Sensors**  
   - Flow sensor (YF-S201) → GPIO interrupt counts pulses.  
   - Pressure sensor (BMP180) → I²C reads pressure values.

2. **Controller (Raspberry Pi Pico W)**  
   - Calculates flow rate (L/min).  
   - Reads pressure (Pa).  
   - Runs leak detection logic.  
   - Controls valve and buzzer.

3. **Leak Detection Logic**  
   - If pressure < 95 kPa → possible leak.  
   - If flow > 10 L/min → possible leak.  
   - If valve closed but flow > 0.5 L/min → confirmed leak.

4. **Actuation**  
   - Valve closes automatically.  
   - Buzzer sounds.  
   - Console prints alert.

5. **IoT/AI Integration (Future Work)**  
   - Send telemetry to cloud (Firebase/ThingSpeak).  
   - AI anomaly detection (Isolation Forest / LSTM).  
   - Dashboard visualization + mobile alerts.

