import os
from playsong import play_audio_with_speed
import pygame


def read_music_info(file_path):
    with open(file_path, "r") as file:
        lines = file.readlines()
    tracks = []
    i=0
    for line in lines:
        if(i==1):
            print(line)
            filename, bpm, key, _= line.strip().split('/')
            bpm = int(bpm)
            tracks.append((filename, bpm, key))
        else:
            i=1
    return tracks


def play_songs_in_order(folder_path):
    # Initialiser Pygame
    pygame.init()

    # Obtenir le nom du dossier
    folder_name = os.path.basename(os.path.normpath(folder_path))
    # Lire les informations des morceaux
    music_info_file = folder_name + ".txt"
    tracks = read_music_info(music_info_file)

    # Jouer les morceaux un par un dans l'ordre
    for filename, bpm, key in tracks:
        file_path = os.path.join(folder_path, filename)
        speed = 1.0  # Vous pouvez ajuster la vitesse ici si nécessaire
        print(f"Playing {filename} at {speed}x speed")
        play_audio_with_speed(file_path, speed)


# Exemple d'utilisation
folder_path = "musictest"  # Remplacez par le chemin de votre dossier
play_songs_in_order(folder_path)
