#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "libs/aubio/src/aubio.h"
//#include "libs/keyfinder/keyfinder.h"
#include "analysis.h"

#define MAX_PATH 1024

void format_bpm(int *bpm) {
    if (*bpm < 80) {
        *bpm *= 2;
    } else if (*bpm > 160) {
        *bpm /= 2;
    }
}

void format_bpm_str(int bpm, char *bpm_str) {
    snprintf(bpm_str, 4, "%03d", bpm);
}

/*void format_key_str(KeyFinder::key_t key, char *key_str) {
    snprintf(key_str, 4, "%02dB", key); // Simple harmonic key formatting example
}*/




#define FRAME_SIZE 512
#define HOP_SIZE 256

int analyze_bpm(const char *file_path) {
    // Initialize variables
    int bpm = 0;
    uint_t read = 0;
    fvec_t *audio_buffer = NULL;
    aubio_source_t *source = NULL;
    aubio_tempo_t *tempo = NULL;

    // Create source
    source = new_aubio_source(file_path, 0, FRAME_SIZE);
    if (!source) {
        fprintf(stderr, "Error opening file %s\n", file_path);
        return 0;
    }

    // Create tempo object
    tempo = new_aubio_tempo("default", FRAME_SIZE, HOP_SIZE, 44100);

    // Create audio buffer
    audio_buffer = new_fvec(FRAME_SIZE);

    // Process audio
    do {
        // Process audio frame
        aubio_source_do(source, audio_buffer, &read);

        // Process tempo
        aubio_tempo_do(tempo, audio_buffer, audio_buffer);

        // Get BPM
        bpm = (int)aubio_tempo_get_bpm(tempo);

    } while (read);

    // Cleanup
    del_aubio_tempo(tempo);
    del_aubio_source(source);
    del_fvec(audio_buffer);

    return bpm;
}
/*KeyFinder::key_t analyze_key(const char *file_path) {
    KeyFinder::AudioData audioData;
    if (!audioData.readFromFile(file_path)) {
        fprintf(stderr, "Could not open file %s for key analysis\n", file_path);
        return KeyFinder::KEY_UNKNOWN;
    }

    KeyFinder::KeyFinder keyFinder;
    KeyFinder::key_t key = keyFinder.keyOfAudio(audioData);

    return key;
}*/

void process_mp3_file(const char *file_path, FILE *output_file) {
    int bpm = analyze_bpm(file_path);
    //KeyFinder::key_t key = analyze_key(file_path);

    format_bpm(&bpm);

    char bpm_str[4];
    //char key_str[4];

    format_bpm_str(bpm, bpm_str);
    //format_key_str(key, key_str);

    fprintf(output_file, "#%s/%s\n", file_path, bpm_str);
}

void process_directory(const char *dir_path, const char *playlist_name) {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(dir_path))) {
        perror("opendir");
        return;
    }

    char output_file_path[MAX_PATH];
    snprintf(output_file_path, MAX_PATH, "src/tracksdata/%s.txt", playlist_name);
    FILE *output_file = fopen(output_file_path, "w");

    if (!output_file) {
        perror("fopen");
        closedir(dir);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char file_path[MAX_PATH];
            snprintf(file_path, MAX_PATH, "%s/%s", dir_path, entry->d_name);

            if (strstr(entry->d_name, ".mp3")) {
                process_mp3_file(file_path, output_file);
            }
        }
    }

    fclose(output_file);
    closedir(dir);
    printf("Fichier créé : %s\n", output_file_path);
}


