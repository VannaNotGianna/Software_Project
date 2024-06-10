from pydub import AudioSegment
import os
import platform
import pygame

def play_audio_at_speed(folder_path, speed_factor):
    # Obtient la liste des fichiers audio dans le dossier
    audio_files = [f for f in os.listdir(folder_path) if f.endswith(".mp3")]

    if not audio_files:
        print("Aucun fichier audio trouvé dans le dossier.")
        return

    # Charge le premier fichier audio
    audio_path = os.path.join(folder_path, audio_files[0])
    sound = AudioSegment.from_file(audio_path)

    # Ajuste la vitesse de lecture
    adjusted_sound = sound.speedup(playback_speed=speed_factor)

    # Exporte le fichier audio ajusté
    adjusted_sound.export("temp_sound.wav", format="wav")

    # Initialise pygame pour la lecture audio
    pygame.mixer.init()
    pygame.mixer.music.load("temp_sound.wav")

    # Joue la musique
    pygame.mixer.music.play()

    # Attend que la musique se termine
    while pygame.mixer.music.get_busy():
        pygame.time.wait(100)

    # Nettoyage
    pygame.mixer.quit()
    os.remove("temp_sound.wav")

# Exemple d'utilisation avec un dossier contenant les fichiers audio
folder_path = "musictest"  # Chemin vers le dossier contenant les fichiers audio
speed_factor = 1.5  # Facteur de vitesse (1.5 pour 150% de la vitesse originale)

play_audio_at_speed(folder_path, speed_factor)

