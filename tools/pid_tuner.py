#!/usr/bin/env python3
"""
@file pid_tuner.py
@author javtl
@brief Aegis-Quad Ground Control - HIL Live PID Tuning Command Injector
@version 1.0
@date 2024-06-15
"""

import sys
import serial
import time

def inject_pid(port, axis, kp, ki, kd):
    try:
        # Abrir puerto compartiendo la velocidad con el dron
        ser = serial.Serial(port, 115200, timeout=1.0)
        time.sleep(1.5) # Espera técnica para la inicialización del bootloader del micro
        
        # Construcción de la trama compacta bajo el estándar definido
        command_payload = f"{axis.upper()},{kp},{ki},{kd}\n"
        
        print(f"[*] Inyectando comando en hardware -> {command_payload.strip()}")
        ser.write(command_payload.encode('utf-8'))
        
        # Leer respuesta de confirmación del firmware
        response = ser.readline().decode('utf-8', errors='ignore').strip()
        if response:
            print(f"[REPORTE DRON]: {response}")
        
        ser.close()
    except Exception as e:
        print(f"[-] Error de conexión en el canal serie: {e}")

if __name__ == "__main__":
    if len(sys.argv) < 5:
        print("[-] Parámetros insuficientes.")
        print("[-] Uso: python3 pid_tuner.py <eje: R|P|Y> <Kp> <Ki> <Kd>")
        print("[-] Ejemplo: python3 pid_tuner.py R 1.35 0.06 0.04")
        sys.exit(1)
        
    target_port = '/dev/ttyACM0' # Modificar según tu asignación en Pop!_OS
    inject_pid(target_port, sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])