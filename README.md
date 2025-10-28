# Sequential Audio Analyzer

Analizador de audio secuencial que calcula espectrogramas mediante STFT (Short-Time Fourier Transform) y detecta el BPM (tempo) de archivos de audio WAV.

## Características

- Procesamiento secuencial: todo el análisis se realiza en un solo proceso
- STFT: Análisis espectral con ventanas Hann (configurable)
- FFT: Implementación Cooley-Tukey in-place
- Detección de BPM: Algoritmo basado en spectral flux y autocorrelación (rango 60-180 BPM)
- Exportación CSV: Espectrograma completo y resultados de análisis
- Estándar C89: Código compatible con ANSI C (C89/C90)

## Estructura del Proyecto

```
.
├── src/
│   ├── main.c          # Orquestación y flujo principal (secuencial)
│   ├── stft.c          # Cálculo de STFT (Short-Time Fourier Transform)
│   ├── wav.c           # Lectura de archivos WAV
│   ├── fft.c           # Transformada rápida de Fourier
│   ├── bpm.c           # Detección de tempo
│   └── window.c        # Funciones de ventaneo
├── include/            # Cabeceras públicas
├── data/               # Archivos de audio WAV
├── results/            # Salida: CSVs del espectrograma y análisis
└── tests/              # Tests unitarios

```

## Compilación

```bash
make
```

El proyecto usa el estándar C89 (ANSI C) para máxima compatibilidad.

O manualmente:
```bash
gcc -std=c89 -O2 -o main src/*.c -lm -I./include
```

## Uso

El ejecutable corre de forma secuencial. Al ejecutarlo, mostrará una lista de archivos WAV disponibles y solicitará la selección de uno para analizar.

```bash
./main
```

### Ejemplo

```bash
./main
```

## Salida

- `results/spectrogram.csv`: Matriz de magnitudes (n_frames × n_bins)
- `results/analysis_results.csv`: BPM detectado y características espectrales

## Arquitectura

### Módulos principales

1. **main.c**: Coordinación general del flujo
   - Selección de archivo de audio
   - Lectura completa del WAV
   - Cálculo del STFT y detección de BPM
   - Escritura de resultados

2. **stft.c**: Cálculo del espectrograma
   - Ventaneo (Hann u otra ventana configurable)
   - Invocación de FFT por frame

3. **bpm.c**: Análisis musical
   - `calculate_spectral_flux()`: Detecta cambios espectrales
   - `calculate_autocorrelation()`: Encuentra periodicidad
   - `find_bpm_from_acf()`: Convierte lag a BPM

## Dependencias

- Biblioteca matemática estándar (`-lm`)
- Compilador C compatible con C89 (gcc, clang)

## Tests

```bash
# Compilar y ejecutar tests unitarios (ejemplo: test_bpm)
gcc tests/test_bpm.c src/bpm.c src/fft.c -o test_bpm -lm -I./include
./test_bpm
```

## Validación

El algoritmo de BPM fue validado con señales sintéticas (ejemplos):
- 60 BPM → detectado ~60.8
- 120 BPM → detectado ~120.2
- 180 BPM → detectado ~178.2

## Licencia

MIT License