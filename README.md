# 🎵 Audio Practice Looper

A professional-grade audio practice application built with JUCE, featuring advanced real-time audio processing, beat detection, and loop manipulation tools.

## ✨ Features

### 🎛️ Core Audio Processing
- **Real-time Time Stretching**: Adjust tempo from 25% to 200% without pitch changes using Rubber Band Library
- **Pitch Shifting**: ±12 semitone range with high-quality algorithms  
- **3-Band EQ**: Professional parametric equalizer with low shelf, peak, and high shelf filters
- **Loop Controls**: Precise loop point editing with snap-to-beat functionality

### 📊 Audio Analysis
- **Beat Detection**: Automatic beat and onset detection using aubio
- **BPM Calculation**: Real-time tempo analysis with confidence metrics
- **Beat Grid Overlay**: Visual beat markers overlaid on waveform display

### 🖥️ User Interface
- **Professional Waveform Display**: Zoom, scroll, and interactive editing
- **Transport Controls**: Full playback control with visual feedback
- **Loop Manipulation**: Quick operations (×½, ×2, ±bars) and beat snapping
- **System Audio Capture**: Record from system audio (Windows WASAPI, Linux PulseAudio)

### 📤 Export Capabilities
- **Multiple Formats**: WAV, MP3, FLAC, OGG support
- **Flexible Options**: Export full file, custom ranges, or loop sections
- **Processing Application**: Choose which effects to apply during export
- **Quality Control**: Configurable sample rates, bit depths, and compression

## 🏗️ Build Requirements

### System Dependencies

**Windows:**
- Visual Studio 2019+ or MinGW-w64
- CMake 3.21+
- Git

**Linux (Ubuntu/Debian):**
```bash
sudo apt update
sudo apt install build-essential cmake git
sudo apt install libasound2-dev libpulse-dev
sudo apt install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev
```

**Linux (Arch/Manjaro):**
```bash
sudo pacman -S base-devel cmake git
sudo pacman -S alsa-lib pulseaudio
sudo pacman -S libx11 libxrandr libxinerama libxcursor
```

### Build Process

1. **Clone the repository:**
```bash
git clone https://github.com/danharris923/audio-practice-looper.git
cd audio-practice-looper
```

2. **Create build directory:**
```bash
mkdir build && cd build
```

3. **Configure with CMake:**
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

4. **Build the application:**
```bash
cmake --build . --config Release
```

5. **Run the application:**
```bash
./bin/AudioPracticeLooper  # Linux
./bin/Release/AudioPracticeLooper.exe  # Windows
```

## 🔧 Dependencies

This project uses CMake with FetchContent to automatically download and build dependencies:

- **[JUCE](https://juce.com/)**: Cross-platform audio framework
- **[Rubber Band Library](https://breakfastquay.com/rubberband/)**: Time-stretching and pitch-shifting
- **[aubio](https://aubio.org/)**: Audio analysis and beat detection
- **[FFmpeg](https://ffmpeg.org/)**: Audio file loading (MP3/WAV/FLAC)

## 🎯 Usage

### Loading Audio
- Click "File → Open" to load audio files (MP3, WAV, FLAC)
- Or drag and drop files onto the waveform view

### Playback Controls
- **Play/Pause**: Spacebar or click the play button
- **Stop**: Stop button or Esc key
- **Seek**: Click anywhere on the waveform

### Loop Controls
- **Set Loop Points**: Drag the green markers on the waveform
- **Quick Adjustments**: Use ×½, ×2, +Bar, -Bar buttons
- **Snap to Beat**: Align loop points to detected beats
- **Loop Here**: Create loop around current playback position

### Tempo & Pitch
- **Tempo Slider**: 25% to 200% speed adjustment
- **Pitch Slider**: ±12 semitones pitch shifting
- Real-time processing with parameter smoothing

### Export
- **File → Export**: Open export dialog
- Choose format, quality, and processing options
- Select export range (full file, custom range, or loop)
- Monitor progress with real-time feedback

## 🏛️ Architecture

### Core Components

```
AudioEngine
├── AudioProcessorGraph (JUCE)
├── AudioFileSource (FFmpeg integration)
├── RubberBandNode (Time/pitch processing)
├── EQNode (3-band parametric EQ)
└── AnalysisWorker (aubio integration)

UI Components
├── WaveformView (Audio visualization)
├── TransportBar (Playback controls)  
├── LoopControls (Loop manipulation)
├── SystemAudioCapture (WASAPI/PulseAudio)
└── ExportDialog (Export functionality)
```

### Thread Safety
- Real-time audio thread with lock-free processing
- Analysis worker runs in separate thread
- Parameter smoothing prevents audio artifacts
- Thread-safe communication using atomics and JUCE message system

## 🧪 Testing

Build and run tests:
```bash
cd build
cmake --build . --target tests
ctest --verbose
```

## 📝 License

This project is licensed under the GPL v3 License - see the [LICENSE](LICENSE) file for details.

The GPL v3 license is chosen to ensure compatibility with all major dependencies:
- JUCE (GPL/Commercial dual license)  
- Rubber Band Library (GPL v2+)
- aubio (GPL v3+)
- FFmpeg (GPL v2+)

## 🤝 Contributing

1. Fork the project
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 🔍 Development

### Code Style
- Follow `.clang-format` configuration
- Use `.clang-tidy` for static analysis
- JUCE coding conventions
- Modern C++20 features

### Debug Build
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

### IDE Setup
The project includes configuration for:
- Visual Studio (Windows)
- CLion (Cross-platform)
- VS Code with C++ extensions

## 📚 Documentation

### API Reference
See individual header files for detailed API documentation.

### Key Classes
- `AudioEngine`: Core audio processing pipeline
- `WaveformView`: Interactive waveform display
- `AnalysisWorker`: Beat detection and analysis
- `ExportEngine`: Audio export functionality

## 🐛 Troubleshooting

### Build Issues
- Ensure CMake 3.21+ is installed
- Check that all system dependencies are available
- Try cleaning build directory: `rm -rf build && mkdir build`

### Audio Issues
- Check audio device settings in system
- Verify ASIO drivers (Windows) or ALSA/PulseAudio (Linux)
- Check sample rate compatibility

### Performance
- Use Release build for optimal performance
- Adjust buffer sizes in audio settings
- Monitor CPU usage during real-time processing

## 📈 Roadmap

- [ ] MIDI controller support
- [ ] Plugin format (VST3/AU) version
- [ ] Advanced analysis (key detection, chord recognition)
- [ ] Cloud sync and collaboration features
- [ ] Mobile companion app

## 🙏 Acknowledgments

- [JUCE Team](https://juce.com/) for the excellent audio framework
- [Breakfast Quay](https://breakfastquay.com/) for Rubber Band Library
- [aubio developers](https://aubio.org/) for audio analysis tools
- [FFmpeg team](https://ffmpeg.org/) for multimedia framework

---

🤖 *This project was developed with assistance from [Claude Code](https://claude.ai/code)*