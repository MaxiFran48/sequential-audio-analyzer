#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wav.h"
#include "common.h"
#include "stft.h"
#include "bpm.h"
#include "sys/time.h"
#include <sys/stat.h>
#include <sys/types.h>


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

    struct timeval t_start, t_end, t_start_compute_stft, t_end_compute_stft, t_start_input, t_end_input, t_start_write_spec, t_end_write_spec;   
    double t_total, t_total_compute_stft, t_total_input, t_total_write_spec;

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


    /* Creamos la ruta para los resultados de esta cancion en particular */
    char aux_path[256];

    /* Copiamos la ruta del audio en la ruta auxiliar */
    strncpy(aux_path, audio_path + 5, 255); /* Copiamos sin el comienzo con "data/" */   

    aux_path[255] = '\0'; /* Aseguramos que la cadena esté terminada en null */
    aux_path[strlen(aux_path) - 4] = '\0'; /* Remover .wav */

    /* Colocamos el nombre del archivo, sin la extension, en la ruta de resultados */
    char* results_path = malloc(256 * sizeof(char));
    sprintf(results_path, "results/%s", aux_path);

    /* Creamos el directorio (ignoramos si ya existe) */
    mkdir(results_path, 0755);

    if (wav_read(audio_path, &wav_file) == -1){
        printf("Error en la lectura del archivo de audio");
        return -1;
    }
    
    n_samples = wav_file.n_samples;

    gettimeofday(&t_start_compute_stft, NULL);

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

    gettimeofday(&t_end_compute_stft, NULL);

    t_total_compute_stft = (t_end_compute_stft.tv_sec - t_start_compute_stft.tv_sec) + (t_end_compute_stft.tv_usec - t_start_compute_stft.tv_usec) / 1e6;
    printf("\nTiempo de computo del STFT: %f segundos\n", t_total_compute_stft);

    printf("\nEspectrograma global terminado (%d ventanas x %d bins)\n", n_frames, n_bins);

    gettimeofday(&t_start_write_spec, NULL);

    /* Guardar el espectrograma en un archivo CSV */
    char* spectrogram_path = malloc(256 * sizeof(char));
    sprintf(spectrogram_path, "%s/spectrogram.csv", results_path);


    FILE *f = fopen(spectrogram_path, "w");
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

    printf("\nArchivo CSV guardado en %s\n", spectrogram_path);
    
    /* Generar CSV y análisis de BPM */
    AnalysisResults* analysis_results;

    char* analysis_results_path = malloc(256 * sizeof(char));
    sprintf(analysis_results_path, "%s/analysis_results.csv", results_path);

    analysis_results = analyze_features_and_bpm(mag, n_frames, n_bins, wav_file.samplerate);
    write_results_to_csv(analysis_results_path, analysis_results, wav_file.samplerate);

    gettimeofday(&t_end, NULL);

    /* Liberar memoria */
    free(analysis_results);
    free(mag);
    wav_free(&wav_file);
    free(samples);
    free(results_path);
    free(spectrogram_path);
    free(analysis_results_path);

    t_total_input = (t_end_input.tv_sec - t_start_input.tv_sec) + (t_end_input.tv_usec - t_start_input.tv_usec) / 1e6;
    t_total_write_spec = (t_end_write_spec.tv_sec - t_start_write_spec.tv_sec) + (t_end_write_spec.tv_usec - t_start_write_spec.tv_usec) / 1e6;
    t_total = (t_end.tv_sec - t_start.tv_sec) + (t_end.tv_usec - t_start.tv_usec) / 1e6;

    printf("\nTiempo total de ejecucion (sin escritura del espectrograma): %f segundos\n", t_total - (t_total_write_spec + t_total_input));

    printf("\nTiempo total de ejecucion: %f segundos\n", t_total);
    
    return 0;
}