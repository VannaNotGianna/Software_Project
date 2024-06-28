// src/App.js
import 'bootstrap/dist/css/bootstrap.min.css';
import React from 'react';
import MusicPlayer from './components/MusicPlayer';
import Navigation from './components/Navigation';

function App() {
  return (
    <div className="App">
      <Navigation />
      <main>
        <MusicPlayer />
      </main>
    </div>
  );
}

export default App;
