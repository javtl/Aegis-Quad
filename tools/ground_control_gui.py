#!/usr/bin/env python3
"""
@file ground_control_gui.py
@author javtl
@brief Aegis-Quad Ground Control Station - Real-Time Graphical Interface
@version 1.0
@date 2024-06-08
"""

import sys
import time
from collections import deque
import matplotlib.pyplot as plt
import matplotlib.animation as animation
# Importamos el motor serie que desarrollamos en el Ticket #18
from telemetry_parser import TelemetryReceiver

# --- Configuración de Ventana de Datos ---
MAX_SAMPLES = 100  # Número de puntos visibles en pantalla simultáneamente
time_history = deque(maxlen=MAX_SAMPLES)
pitch_history = deque(maxlen=MAX_SAMPLES)
roll_history = deque(maxlen=MAX_SAMPLES)

# Inicializar receptor serie (Ajusta el puerto '/dev/ttyACM0' o '/dev/ttyUSB0')
receiver = TelemetryReceiver(port='/dev/ttyACM0', baudrate=115200)

# Configuración estética de la ventana de gráficos de Matplotlib
plt.style.use('dark_background')
fig, (ax_pitch, ax_roll) = plt.subplots(2, 1, sharex=True, figsize=(10, 6))
fig.canvas.manager.set_window_title('Aegis-Quad - Real-Time Ground Control Station')

# Líneas de datos dinámicas (Inicialmente vacías)
line_pitch, = ax_pitch.plot([], [], color='#ff4757', lw=2, label='Pitch (Cabeceo)')
line_roll, = ax_roll.plot([], [], color='#2ed573', lw=2, label='Roll (Alabeo)')

def init_gui():
    """Configuración inicial de los ejes del gráfico"""
    ax_pitch.set_title("Eje de Pitch (Inclinación Frontal)", color='#ffffff', loc='left', pad=10)
    ax_pitch.set_ylim(-45, 45) # El dron limita su ángulo máximo a 30° por software
    ax_pitch.grid(True, linestyle='--', alpha=0.3)
    ax_pitch.set_ylabel("Grados (°)")
    ax_pitch.legend(loc='upper right')

    ax_roll.set_title("Eje de Roll (Inclinación Lateral)", color='#ffffff', loc='left', pad=10)
    ax_roll.set_ylim(-45, 45)
    ax_roll.grid(True, linestyle='--', alpha=0.3)
    ax_roll.set_ylabel("Grados (°)")
    ax_roll.set_xlabel("Tiempo relativo (Muestras)")
    ax_roll.legend(loc='upper right')
    
    return line_pitch, line_roll

def update_gui(frame):
    """Bucle de renderizado animado (Corre a intervalos regulares de tiempo)"""
    # Vaciar la cola de datos acumulada en el hilo serie
    while not receiver.data_queue.empty():
        raw_frame = receiver.data_queue.get()
        parsed_data = receiver.parse_frame(raw_frame)
        
        if parsed_data:
            # Añadir marcas de tiempo relativas y ángulos convertidos a las colas de memoria
            time_history.append(parsed_data['time_ms'] / 1000.0) # Convertir a segundos
            pitch_history.append(parsed_data['pitch'])
            roll_history.append(parsed_data['roll'])

    # Si hay datos disponibles, actualizar la estructura geométrica de las líneas
    if time_history:
        x_indices = list(range(len(time_history))) # Índices de muestras secuenciales
        
        line_pitch.set_data(x_indices, list(pitch_history))
        line_roll.set_data(x_indices, list(roll_history))
        
        # Ajustar dinámicamente el eje X horizontal para simular el desplazamiento continuo
        ax_pitch.set_xlim(0, MAX_SAMPLES - 1)
        ax_roll.set_xlim(0, MAX_SAMPLES - 1)

    return line_pitch, line_roll

if __name__ == "__main__":
    # Arrancar el hilo de ingesta serie en segundo plano
    receiver.start()

    # Configurar la animación de Matplotlib (blit=True optimiza el repintado usando la GPU)
    ani = animation.FuncAnimation(
        fig, update_gui, init_func=init_gui, blit=True, interval=30, cache_frame_data=False
    )

    print("[*] Desplegando interfaz gráfica interactiva. Cierra la ventana para salir.")
    try:
        plt.tight_layout()
        plt.show()
    except KeyboardInterrupt:
        pass
    finally:
        # Garantizar el cierre seguro de los hilos de hardware al salir de la app
        receiver.stop()