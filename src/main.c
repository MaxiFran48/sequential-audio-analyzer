#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wav.h"
#include "common.h"
#include "stft.h"
#include "bpm.h"
#include "sys/time.h"


int main (int argc, char* argv[]) {
    WAVFile wav_file;
    int n_samples;
    float *samples;
    int n_frames, n_bins;
    float *mag;
    int i, k;

    char* wav_list_path = "data/lista.wavs.txt";
    char files[MAX_FILES][MAX_PATH];
    int audio_index = 0;
    int files_count;

    struct timeval t_start, t_end, t_start_input, t_end_input, t_start_write_spec, t_end_write_spec;
    gettimeofday(&t_start, NULL);

    /* Cargamos el archivo con la lista de audios */
    files_count = load_wav_list(wav_list_path, files);

    /* Verificar si se pudo cargar el archivo */
    if (files_count < 0) {
        fprintf(stderr, "Error: No se pudo abrir el archivo %s\n", wav_list_path);
        return -1;
    } else if (files_count == 0) {
        printf("Advertencia: No se encontraron archivos en %s\n", wav_list_path);
        return -1;
    }

    /* Mostramos las canciones y pedimos que elija una al usuario */
    printf("Lista de audios:\n");
    
    for (i = 0; i < files_count; i++) {
        printf("%d. %s\n", i+1, files[i]);
    }

    gettimeofday(&t_start_input, NULL);

    printf("\nPor favor, ingrese el numero de audio para analizar:\n");
    scanf("%d", &audio_index);

    while (audio_index < 0 || audio_index > files_count){
        printf("\nNumero invalido, por favor, intente nuevamente:\n");
        scanf("%d", &audio_index);
    }

    gettimeofday(&t_end_input, NULL);

    char* audio_path = files[audio_index-1];

    if (wav_read(audio_path, &wav_file) == -1){
        printf("Error en la lectura del archivo de audio");
        return -1;
    }
    
    n_samples = wav_file.n_samples;
    
    /* Alocar memoria */
    samples = malloc(n_samples * sizeof(float));

    if (!samples) {
        fprintf(stderr, "Error: No se pudo alocar memoria para samples\n");
        return -1;
    }

    /* En rank 0, copiar los datos del wav_file */
    memcpy(samples, wav_file.samples, n_samples * sizeof(float));
    
    /* Calcular parámetros del STFT */
    n_frames = (n_samples - DEFAULT_N) / DEFAULT_HOP + 1;
    n_bins = DEFAULT_N / 2 + 1;

    /* Computar STFT */
    mag = compute_stft(samples, n_frames, n_bins);

    if (!mag) {
        fprintf(stderr, "Error: No se pudo alocar memoria para mag\n");
        return -1;
    }


    /* Generar CSV y análisis de BPM */
    FILE *f;
    AnalysisResults* analysis_results;
    
    printf("\nEspectrograma global recibido (%d ventanas x %d bins)\n", n_frames, n_bins);

    gettimeofday(&t_start_write_spec, NULL);

    f = fopen("results/spectrogram.csv", "w");
    if (!f) {
        perror("No se pudo crear el archivo CSV");
        return -1;
    }

    for (i = 0; i < n_frames; i++) {
        for (k = 0; k < n_bins; k++) {
            fprintf(f, "%.6f", mag[i * n_bins + k]);
            if (k < n_bins - 1)
                fprintf(f, ",");
        }
        fprintf(f, "\n");
    }

    fclose(f);

    gettimeofday(&t_end_write_spec, NULL);

    printf("\nArchivo CSV guardado en results/spectrogram.csv\n");
    
    /* Calcular BPM y características */
    analysis_results = analyze_features_and_bpm(mag, n_frames, n_bins, wav_file.samplerate);
    write_results_to_csv("results/analysis_results.csv", analysis_results, wav_file.samplerate);

    gettimeofday(&t_end, NULL);

    /* Liberar memoria */
    free(analysis_results);
    free(mag);
    wav_free(&wav_file);
    free(samples);

    printf("\nTiempo total de ejecucion: %.2f segundos\n", (t_start.tv_sec - t_end_input.tv_sec) + (t_start.tv_usec - t_end_input.tv_usec) / 1e6 - ((t_start_write_spec.tv_sec - t_end_write_spec.tv_sec) + (t_start_write_spec.tv_usec - t_end_write_spec.tv_usec) / 1e6) - ((t_start_input.tv_sec - t_end_input.tv_sec) + (t_start_input.tv_usec - t_end_input.tv_usec) / 1e6));

    return 0;
}