import sys
from mutagen.mp3 import MP3

def get_mp3_duration(file_path):
    audio = MP3(file_path)
    return audio.info.length

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python get_duration.py <mp3_file>")
        sys.exit(1)

    file_path = sys.argv[1]
    duration = get_mp3_duration(file_path)
    print(duration)
