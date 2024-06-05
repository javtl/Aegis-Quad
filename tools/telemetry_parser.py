#!/usr/bin/env python3
"""
@file telemetry_parser.py
@author javtl
@brief Aegis-Quad Ground Control - High-Speed Serial Ingestion Engine
@version 1.0
@date 2024-06-05
"""

import serial
import threading
import queue
import time

class TelemetryReceiver:
    def __init__(self, port='/dev/ttyUSB0', baudrate=115200):
        self.port = port
        self.baudrate = baudrate
        self.data_queue = queue.Queue()
        self.running = False
        self.thread = None
        self.ser = None

    def start(self):
        try:
            self.ser = serial.Serial(self.port, self.baudrate, timeout=1.0)
            self.ser.flushInput()
            self.running = True
            self.thread = threading.Thread(target=self._read_serial, daemon=True)
            self.thread.start()
            print(f"[*] Conectado con éxito al puerto {self.port} a {self.baudrate} baudios.")
        except serial.SerialException as e:
            print(f"[-] Error crítico al abrir el puerto serie: {e}")
            self.running = False

    def _read_serial(self):
        while self.running:
            if self.ser.in_waiting > 0:
                try:
                    # Leer línea cruda del hardware serie
                    raw_line = self.ser.readline()
                    # Decodificar obviando caracteres corruptos por ruido en la línea
                    decoded_line = raw_line.decode('utf-8', errors='ignore').strip()
                    
                    if decoded_line:
                        self.data_queue.put(decoded_line)
                except Exception as e:
                    print(f"[!] Error de lectura en el hilo serie: {e}")
            time.sleep(0.001) # Liberar CPU para evitar consumo del 100% de un núcleo

    def parse_frame(self, frame_str):
        """
        Parsea el protocolo compacto: Time,Pitch,Roll,Throttle,Voltage,State
        """
        try:
            parts = frame_str.split(',')
            if len(parts) == 6:
                return {
                    "time_ms": int(parts[0]),
                    "pitch": float(parts[1]),
                    "roll": float(parts[2]),
                    "throttle": int(parts[3]),
                    "voltage": float(parts[4]),
                    "state": int(parts[5])
                }
        except ValueError:
            # Captura errores si la trama se cortó a la mitad o contiene basura
            pass
        return None

    def stop(self):
        self.running = False
        if self.thread:
            self.thread.join(timeout=2.0)
        if self.ser and self.ser.is_open:
            self.ser.close()
        print("[*] Motor de telemetría detenido y puerto serie liberado.")

# --- Código de prueba de ejecución directa ---
if __name__ == "__main__":
    # Ajusta el puerto según detecte tu Pop!_OS (ej: /dev/ttyACM0 o /dev/ttyUSB0)
    receiver = TelemetryReceiver(port='/dev/ttyACM0', baudrate=115200)
    receiver.start()

    print("[*] Escuchando telemetría en tiempo real... Presiona Ctrl+C para salir.")
    try:
        while receiver.running:
            if not receiver.data_queue.empty():
                raw_frame = receiver.data_queue.get()
                parsed_data = receiver.parse_frame(raw_frame)
                
                if parsed_data:
                    print(f"[TELEMETRÍA] T: {parsed_data['time_ms']}ms | P: {parsed_data['pitch']}° | R: {parsed_data['roll']}° | Bat: {parsed_data['voltage']}V | Estado: {parsed_data['state']}")
            time.sleep(0.01)
    except KeyboardInterrupt:
        print("\n[-] Cancelado por el usuario.")
    finally:
        receiver.stop()