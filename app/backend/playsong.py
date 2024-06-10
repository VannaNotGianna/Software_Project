import pygame
from pydub import AudioSegment
import os


def play_audio_with_speed(file_path, speed=1.0):
    # Charger le fichier audio
    #audio = AudioSegment.from_file(file_path)

    # Appliquer le coefficient de vitesse
    #new_audio = audio.speedup(playback_speed=speed)

    # Exporter l'audio modifié en format wav temporaire
    #temp_file = "temp_audio.wav"
    #new_audio.export(temp_file, format="wav")

    # Initialiser pygame
    pygame.mixer.init()

    # Charger l'audio modifié avec pygame
    pygame.mixer.music.load(file_path)

    # Jouer l'audio
    pygame.mixer.music.play()

    # Attendre que la musique se termine
    while pygame.mixer.music.get_busy():
        pygame.time.Clock().tick(10)

    # Supprimer le fichier temporaire
    os.remove(temp_file)


# Exemple d'utilisation
file_path = "musictest/Belly Dancer.mp3"  # Remplacez par le chemin de votre fichier audio
speed = 1.5  # Coefficient de vitesse (1.0 pour vitesse normale)
#play_audio_with_speed(file_path, speed)
