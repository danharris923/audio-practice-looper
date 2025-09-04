# 🚀 Quick Start Guide - Audio Practice Looper

## Option 1: Automated Build (Recommended)

```bash
# Clone the repository
git clone https://github.com/danharris923/audio-practice-looper.git
cd audio-practice-looper

# Run automated build script
python simple_build.py
```

The script will:
- ✅ Check for required tools (CMake, compiler)
- ✅ Show you what to install if missing
- ✅ Automatically build if tools are available

## Option 2: Manual Build

### Windows (Visual Studio)

1. **Install Prerequisites:**
   - [CMake 3.21+](https://cmake.org/download/)
   - [Visual Studio 2019+ Community](https://visualstudio.microsoft.com/downloads/) or Build Tools

2. **Build Commands:**
```cmd
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -A x64
cmake --build . --config Release
```

3. **Run:**
```cmd
.\bin\Release\AudioPracticeLooper.exe
```

### Linux (Ubuntu/Debian)

1. **Install Prerequisites:**
```bash
sudo apt update
sudo apt install build-essential cmake git pkg-config
sudo apt install libasound2-dev libpulse-dev libpulse-simple0
sudo apt install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev
sudo apt install libfreetype6-dev libgl1-mesa-dev
```

2. **Build Commands:**
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
```

3. **Run:**
```bash
./bin/AudioPracticeLooper
```

## What You'll See When It Runs

The application will open with a professional audio interface featuring:

```
┌─────────────────────────────────────────────────────────────┐
│                   🎵 Audio Practice Looper                   │
├─────────────────────────────────────────────────────────────┤
│ File: [Load Audio File] │ ♪ BPM: ---  │ 🔊 Volume: ████░░░   │
├─────────────────────────────────────────────────────────────┤
│                    WAVEFORM DISPLAY                         │
│  ╭───────────────────────────────────────────────────────╮  │
│  │ ▁▂▃▅▇█▇▅▃▂▁▂▃▅▇██▇▅▃▂▁▂▃▄▅▆▇▆▅▄▃▂▁▂▃▄▅▆▇█▇▆▅▄▃▂▁ │  │
│  │         ♦    ♦    ♦    ♦    ♦    ♦    ♦    ♦       │  │
│  │               ▲ playback    [loop]                  │  │
│  ╰───────────────────────────────────────────────────────╯  │
├─────────────────────────────────────────────────────────────┤
│ ▶️ Play │ ⏸️ Pause │ ⏹️ Stop │ 🔄 Loop │ 00:00 / 03:24   │
├─────────────────────────────────────────────────────────────┤
│ Tempo: 100% ████████████████████  Pitch: +0 ██████████     │
│ EQ: Lo████░░ Mid██████░░ Hi█████░░                          │
├─────────────────────────────────────────────────────────────┤
│ [×½] [×2] [-Bar] [+Bar] [◄Beat] [Beat►] [◄►Beat]           │
└─────────────────────────────────────────────────────────────┘
```

## First Steps

1. **Load Audio File**: Click "File → Open" or drag-drop an MP3/WAV/FLAC file
2. **Start Playback**: Press Space or click Play button
3. **Adjust Tempo**: Use tempo slider (25% to 200%)
4. **Set Loop**: Drag on waveform to set loop points
5. **Practice**: Use loop controls to fine-tune practice sections

## Troubleshooting

### Build Fails?
```bash
# Check what's missing
python simple_build.py

# Common fixes:
# - Install CMake from cmake.org
# - Install Visual Studio Build Tools (Windows)
# - Install build-essential (Linux)
```

### Audio Issues?
- Check audio device settings in system
- Try different sample rates (44.1kHz, 48kHz)
- Verify audio drivers are up to date

### Performance Issues?
- Use Release build (not Debug)
- Close other audio applications
- Check CPU usage during playback

## Demo Mode

Can't build right now? Try the demo:
```bash
python ascii_demo.py
```

Shows the full application interface and features in action!

## Support

- 📖 Full docs: [README.md](README.md)
- 🐛 Issues: [GitHub Issues](https://github.com/danharris923/audio-practice-looper/issues)
- 💬 Discussions: [GitHub Discussions](https://github.com/danharris923/audio-practice-looper/discussions)

---

**Ready to transform your practice sessions with professional audio tools!** 🎵