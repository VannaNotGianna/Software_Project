import os
import re
from tqdm import tqdm
import essentia
import essentia.standard as es


def analyze_music_folder(folder_path):
    # Obtenir le nom du dossier
    folder_name = os.path.basename(os.path.normpath(folder_path))

    # Initialiser une liste pour stocker les informations des morceaux
    tracks_info = []

    # Obtenir la liste des fichiers mp3 dans le dossier
    files = [f for f in os.listdir(folder_path) if f.endswith(".mp3")]

    # Parcourir tous les fichiers dans le dossier avec tqdm pour la barre de progression
    for filename in tqdm(files, desc="Analyzing tracks", unit="track"):
        file_path = os.path.join(folder_path, filename)

        # Charger le fichier audio
        loader = es.MonoLoader(filename=file_path)
        audio = loader()

        # Extraire le tempo (BPM) et les beats
        rhythm_extractor = es.RhythmExtractor2013(method="multifeature")
        bpm, beat_times, _, _, _ = rhythm_extractor(audio)

        # Filtrer les BPM pour qu'ils soient entre 80 et 160
        while(bpm < 80):
            bpm = bpm*2
        while(bpm > 160)
            bpm = bpm /2

        # Extraire la clé musicale (Key)
        key_extractor = es.KeyExtractor()
        key, scale, strength = key_extractor(audio)
        harmonic_key = convert_to_harmonic_key(key, scale)

        # Obtenir le timecode du premier temps (en secondes)
        first_beat_time = beat_times[0] if len(beat_times) > 0 else 0.0

        # Ajouter les informations du morceau à la liste
        track_info = (int(bpm_str), harmonic_key, f"{filename}/{bpm_str}/{harmonic_key}/{first_beat_time:.2f}")
        tracks_info.append(track_info)

    # Trier les informations des morceaux par BPM croissant et par Key si les BPM sont identiques
    tracks_info.sort()

    # Créer le fichier texte avec le nom du dossier
    output_file = f"{folder_name}.txt"
    with open(output_file, "w") as file:
        file.write(folder_name + "\n")
        for _, _, track_info in tracks_info:
            file.write(track_info + "\n")

    print(f"Fichier {output_file} créé avec succès.")


def convert_to_harmonic_key(key, scale):
    # Dictionnaire de conversion vers la notation harmonique
    key_mapping = {
        'C': '01', 'C#': '02', 'D': '03', 'D#': '04', 'E': '05',
        'F': '06', 'F#': '07', 'G': '08', 'G#': '09', 'A': '10',
        'A#': '11', 'B': '12'
    }
    scale_mapping = {
        'major': 'B', 'minor': 'A'
    }

    key_str = key_mapping.get(key, "00")
    scale_str = scale_mapping.get(scale, "X")

    return f"{key_str}{scale_str}"


# Exemple d'utilisation
#folder_path = "musictest"
#analyze_music_folder(folder_path)
