#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stft.h"
#include "common.h"
#include "window.h"
#include "fft.h"

float* compute_stft(float* samples, int n_frames, int n_bins) {
    
    float* mag;
    int i, k;
    
    mag = malloc(n_frames * n_bins * sizeof(float));
    
    if (!mag) {
        return NULL;
    }

    for (i = 0; i < n_frames; i++) {
        float real[DEFAULT_N];
        float imaginary[DEFAULT_N];
        float r, imv;
        float* frame;

        frame = malloc(DEFAULT_N * sizeof(float));

        /* Extraer ventana actual */
        memcpy(frame, samples + i * DEFAULT_HOP, sizeof(float) * DEFAULT_N);
        
        /* Aplicar ventana de Hann */
        window_apply(frame, DEFAULT_N, WIN_HANN);

        /* Preparar arrays para FFT */
        for (k = 0; k < DEFAULT_N; k++) {
            real[k] = frame[k];
            imaginary[k] = 0.0f;
        }

        /* Aplicar FFT */
        fft_inplace(real, imaginary, DEFAULT_N);

        /* Calcular magnitudes */
        for (k = 0; k < n_bins; k++) {
            r = real[k];
            imv = imaginary[k];
            mag[i * n_bins + k] = (float)sqrt(r * r + imv * imv);
        }
    }

    return mag;
}
