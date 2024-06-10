import os
import random
import pygame
import signal
from pydub import AudioSegment
import sys


def select_and_play_two_sounds(folder_path, ratio1, ratio2, selected1, selected2):
    # Initialiser pygame mixer
    pygame.mixer.init()

    # Obtenir la liste des fichiers mp3 dans le dossier
    files = [f for f in os.listdir(folder_path) if f.endswith(".mp3")]

    if len(files) < 2:
        print("Le dossier doit contenir au moins deux fichiers audio.")
        return

    # Sélectionner deux fichiers audio au hasard
    selected_files = random.sample(files, 2)
    file1_path = os.path.join(folder_path, selected1)
    file2_path = os.path.join(folder_path, selected2)

    sound1 = AudioSegment.from_file(file1_path)
    sound2 = AudioSegment.from_file(file2_path)


    # Ajuste la vitesse de lecture
    adjusted_sound1 = speedup(sound1, 1.2, 150)
    adjusted_sound1.export("temp_sound1.mp3", format="mp3")
    adjusted_sound2 = speedup(sound2, 1.1, 150)
    adjusted_sound2.export("temp_sound2.mp3", format="mp3")
    # Charger les fichiers audio

    #pygame.mixer.music.load("temp_sound.wav")
    sound1 = pygame.mixer.Sound("temp_sound1.mp3")
    sound2 = pygame.mixer.Sound("temp_sound2.mp3")

    # Jouer les deux sons simultanément
    sound1.play()
    sound2.play()

    # Attendre que la lecture des deux musiques soit terminée
    pygame.time.wait(round(max(sound1.get_length(), sound2.get_length()) * 1000))

    print(f"Lecture synchronisée de {selected1} et {selected2} terminée.")


def signal_handler(sig, frame):
    print("Arrêt du programme.")
    pygame.mixer.quit()
    sys.exit(0)

def get_bpm_from_text_file(file_path, mp3_file_name):
    try:
        # Ouvre le fichier texte en mode lecture
        with open(file_path, 'r') as file:
            # Lit chaque ligne du fichier
            for line in file:
                # Vérifie si la ligne contient le nom du fichier MP3
                if mp3_file_name in line:
                    # Divise la ligne en utilisant '/' comme séparateur
                    parts = line.strip().split('/')
                    # Si la ligne est divisée en au moins 3 parties (nom du fichier, BPM, KEY)
                    if len(parts) >= 3:
                        # Extrait et renvoie le BPM
                        return int(parts[1])
                    else:
                        print("Le fichier texte ne contient pas d'informations BPM valides.")
                        return None
            # Si le nom du fichier MP3 n'est pas trouvé dans le fichier texte
            print("Le nom du fichier MP3 n'a pas été trouvé dans le fichier texte.")
            print(mp3_file_name)
            return None
    except Exception as e:
        print("Une erreur s'est produite lors de la lecture du fichier texte:", e)
        return None

def get_average(bpm1,bpm2):
    average = (bpm1+bpm2)/2
    print(average)
    ratio1 = average/bpm1
    ratio2 = average/bpm2
    return (ratio1, ratio2)

# Exemple d'utilisation
folder_path = "musictest"

files = [f for f in os.listdir(folder_path) if f.endswith(".mp3")]
selected_files = random.sample(files, 2)


# Exemple d'utilisation
file_path = "musictest.txt"  # Chemin vers le fichier texte
audio_files = [f for f in os.listdir(folder_path) if f.endswith(".mp3")]
mp3_file_name = "Belly Dancer.mp3"  # Nom du fichier MP3 dont vous voulez extraire le BPM
bpm1 = get_bpm_from_text_file(file_path, selected_files[0])
bpm2 = get_bpm_from_text_file(file_path, selected_files[1])
(r1,r2)=get_average(bpm1,bpm2)
print(selected_files[0])
print(bpm1)
print("ratio:" and r1)
print(selected_files[1])
print(bpm2)
print("ratio:" and r2)





# Définir le gestionnaire de signal pour intercepter Ctrl+C
signal.signal(signal.SIGINT, signal_handler)

# Appeler la fonction principale
select_and_play_two_sounds(folder_path, 1, 1, selected_files[0], selected_files[1])
