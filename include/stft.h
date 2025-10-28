#ifndef STFT_H
#define STFT_H

/**
 * Calcula el STFT (Short-Time Fourier Transform) de un conjunto de frames
 * asignados a este proceso.
 * 
 * @param samples Array completo de muestras de audio
 * @param n_frames Cantidad total de ventanas a procesar
 * @param n_bins Cantidad de bins de frecuencia (N/2 + 1)
 * @return Array con las magnitudes calculadas (n_frames * n_bins elementos)
 */
float* compute_stft(float* samples, int n_frames, int n_bins);


#endif
