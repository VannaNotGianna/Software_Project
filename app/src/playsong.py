import signal
import sys
import pygame

# Initialisation de pygame
pygame.mixer.init()

# Fonction pour gérer l'interruption du signal
def signal_handler(sig, frame):
    pygame.mixer.music.stop()
    pygame.mixer.quit()
    sys.exit(0)

# Enregistrement du signal pour gérer la fermeture propre
signal.signal(signal.SIGTERM, signal_handler)

def play_song(playlist_path, track_name):
    try:
        file_path = f"{playlist_path}/{track_name}.mp3"
        pygame.mixer.music.load(file_path)
        pygame.mixer.music.play()
        while pygame.mixer.music.get_busy():
            pygame.time.Clock().tick(10)  # Petit délai pour éviter de consommer trop de CPU
    except Exception as e:
        print(f"Error playing {file_path}: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python3 playsong.py <playlist_path> <track_name>")
        sys.exit(1)

    playlist_path = sys.argv[1]
    track_name = sys.argv[2]

    play_song(playlist_path, track_name)
