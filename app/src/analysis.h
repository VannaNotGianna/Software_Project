#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "libs/aubio/src/aubio.h"
//#include "libs/keyfinder/keyfinder.h"


#ifndef ANALYSIS_H
#define ANALYSIS_H




void process_directory(const char *dir_path, const char *playlist_name);
void process_mp3_file(const char *file_path, FILE *output_file);
//void format_key_str(char *key, char *key_str);
void format_bpm_str(int bpm, char *bpm_str);
void format_bpm(int *bpm);
int analyze_bpm(const char *file_path);


#endif // ANALYSIS_H
