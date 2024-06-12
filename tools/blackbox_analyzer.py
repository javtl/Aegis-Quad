#!/usr/bin/env python3
"""
@file blackbox_analyzer.py
@author javtl
@brief Aegis-Quad Ground Control - Post-Flight Blackbox Log Analyzer
@version 1.0
@date 2024-06-12
"""

import os
import sys
import pandas as pd
import matplotlib.pyplot as plt

def analyze_log(file_path):
    if not os.path.exists(file_path):
        print(f"[-] Error: El archivo de log '{file_path}' no existe.")
        return

    print(f"[*] Cargando telemetría local de la Blackbox: {file_path}")
    
    # 1. Ingesta y parseo automático del CSV mediante Pandas
    try:
        df = pd.read_csv(file_path)
    except Exception as e:
        print(f"[-] Error al leer la estructura de datos: {e}")
        return

    # Validar que el archivo contenga las columnas requeridas por el firmware
    required_cols = ["Time_ms", "Pitch", "Roll", "Throttle", "Voltage", "State"]
    if not all(col in df.columns for col in required_cols):
        print("[-] Error: El archivo CSV no tiene el formato estándar de Aegis-Quad.")
        return

    # 2. Computación de Métricas Analíticas
    total_flight_time_sec = (df["Time_ms"].iloc[-1] - df["Time_ms"].iloc[0]) / 1000.0
    max_pitch = df["Pitch"].abs().max()
    max_roll = df["Roll"].abs().max()
    min_voltage = df["Voltage"].min()
    avg_throttle = df["Throttle"].mean()

    # Mapeo estético de estados de la FSM para el reporte
    state_mapping = {0: "BOOT", 1: "DISARMED", 2: "CALIBRATING", 3: "ARMED", 4: "FAILSAFE"}
    detected_states = [state_mapping.get(s, "UNKNOWN") for s in df["State"].unique()]

    print("\n" + "="*40)
    print("       AEGIS-QUAD FLIGHT REPORT       ")
    print("="*40)
    print(f"• Duración de Registro : {total_flight_time_sec:.2f} segundos")
    print(f"• Estados de la FSM    : {', '.join(detected_states)}")
    print(f"• Aceleración Promedio : {avg_throttle:.1f} us")
    print(f"• Desviación Máx Pitch : {max_pitch:.2f}°")
    print(f"• Desviación Máx Roll  : {max_roll:.2f}°")
    print(f"• Tensión Mínima Bat   : {min_voltage:.2f}V " + ("⚠️ ¡ALERTA BAJO VOLTAJE!" if min_voltage < 10.5 else "✓ OK"))
    print("="*40 + "\n")

    # 3. Renderizado de Gráficos de Alta Resolución para Diagnóstico
    plt.style.use('dark_background')
    fig, (ax1, ax2, ax3) = plt.subplots(3, 1, sharex=True, figsize=(12, 8))
    fig.canvas.manager.set_window_title(f'Aegis-Quad Flight Analytics - {os.path.basename(file_path)}')

    # Conversión del eje X a unidades de tiempo legibles (segundos relacionales)
    time_sec = (df["Time_ms"] - df["Time_ms"].iloc[0]) / 1000.0

    # Subplot 1: Dinámica de Orientación (Filtro Complementario / PIDs)
    ax1.plot(time_sec, df["Pitch"], color='#ff4757', alpha=0.9, label='Pitch (Cabeceo)')
    ax1.plot(time_sec, df["Roll"], color='#2ed573', alpha=0.9, label='Roll (Alabeo)')
    ax1.grid(True, linestyle=':', alpha=0.4)
    ax1.set_ylabel("Ángulo (°)")
    ax1.set_title("Dinámica de Actitud y Estabilización", loc='left', color='#ffffff')
    ax1.legend(loc='upper right')

    # Subplot 2: Entrada de Motor (Throttle del Piloto)
    ax2.plot(time_sec, df["Throttle"], color='#ffa502', lw=1.5, label='Consigna Throttle')
    ax2.grid(True, linestyle=':', alpha=0.4)
    ax2.set_ylabel("Pulso (µs)")
    ax2.set_title("Demanda Energética del Sistema (Mando)", loc='left', color='#ffffff')
    ax2.legend(loc='upper right')

    # Subplot 3: Curva de Descarga Química de la Batería
    ax3.plot(time_sec, df["Voltage"], color='#1e90ff', lw=1.5, label='Voltaje LiPo')
    ax3.axhline(y=10.5, color='#ff6b81', linestyle='--', alpha=0.7, label='Umbral Failsafe (10.5V)')
    ax3.grid(True, linestyle=':', alpha=0.4)
    ax3.set_ylabel("Voltaje (V)")
    ax3.set_xlabel("Tiempo de Vuelo (Segundos)")
    ax3.set_title("Curva de Rendimiento Eléctrico", loc='left', color='#ffffff')
    ax3.legend(loc='upper right')

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    # Verificación de argumento de entrada por CLI
    if len(sys.argv) < 2:
        print("[-] Uso correcto: python3 blackbox_analyzer.py <ruta_al_archivo_log.csv>")
        print("[-] Ejemplo: python3 blackbox_analyzer.py logs/LOG_001.CSV")
        sys.exit(1)

    analyze_log(sys.argv[1])