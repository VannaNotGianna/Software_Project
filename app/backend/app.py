from flask import Flask, request, jsonify
from test import play_song

app = Flask(__name__)

@app.route('/api/normal-play', methods=['POST'])
def normal_play():
    data = request.get_json()
    playlist = data['playlist']
    # Ajoutez ici le code pour le mode de lecture normal
    return jsonify({"status": "Normal play started for playlist: " + playlist})

@app.route('/api/intelligent-play', methods=['POST'])
def intelligent_play():
    data = request.get_json()
    playlist = data['playlist']
    # Ajoutez ici le code pour le mode de lecture intelligent
    return jsonify({"status": "Intelligent play started for playlist: " + playlist})

@app.route('/api/play-song', methods=['POST'])
def play_song_api():
    data = request.get_json()
    song_name = data['song_name']
    result = play_song(song_name)
    return jsonify({"result": result})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
