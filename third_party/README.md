# Third Party Dependencies

This project uses several third-party libraries. All dependencies are automatically fetched and built using CMake FetchContent.

## Dependencies

### JUCE Framework (v7.0.12)
- **License**: GPL v3 / Commercial
- **Purpose**: Cross-platform audio application framework
- **Repository**: https://github.com/juce-framework/JUCE
- **Build Notes**: 
  - Automatically fetched via CMake
  - Only required modules are built
  - Tools, examples, and extras are disabled

### Rubber Band Audio Time Stretcher (v4.0.0)
- **License**: GPL v2 or later
- **Purpose**: Real-time time-stretching and pitch-shifting
- **Repository**: https://github.com/breakfastquay/rubberband
- **Build Notes**:
  - Built as static library
  - FFTW3 support enabled on Linux if available
  - Vamp and LADSPA plugins disabled
  - Uses built-in FFT on Windows

### aubio Audio Analysis Library (v0.4.9)
- **License**: GPL v3
- **Purpose**: Beat detection, onset detection, tempo analysis
- **Repository**: https://github.com/aubio/aubio
- **Build Notes**:
  - Manual CMake integration (aubio uses waf)
  - Minimal configuration for beat/onset detection only
  - External audio format support disabled
  - Built as static library

### FFmpeg (System Install Required)
- **License**: GPL v2+ / LGPL v2.1+
- **Purpose**: Audio file decoding/encoding (MP3, WAV, FLAC)
- **Repository**: https://github.com/FFmpeg/FFmpeg
- **Build Notes**:
  - System installation required
  - Detected via pkg-config or manual search
  - Required components: libavcodec, libavformat, libavutil, libswresample
  - Minimum versions: avcodec>=58.0.0, avformat>=58.0.0, avutil>=56.0.0

### Catch2 (Testing Framework)
- **License**: BSL-1.0
- **Purpose**: Unit testing framework
- **Repository**: https://github.com/catchorg/Catch2
- **Build Notes**:
  - Fetched automatically for test builds
  - Header-only library
  - Only included in test targets

## Platform-Specific Build Requirements

### Windows
- Visual Studio 2019 or later with C++20 support
- FFmpeg can be installed via vcpkg or manually
- Windows SDK 10.0.19041.0 or later for WASAPI support

### Linux
- GCC 10+ or Clang 11+ with C++20 support
- System packages required:
  ```bash
  # Ubuntu/Debian
  sudo apt install libavcodec-dev libavformat-dev libavutil-dev libswresample-dev
  sudo apt install libfftw3-dev  # Optional, for better Rubber Band performance
  sudo apt install libasound2-dev  # For ALSA support
  sudo apt install libpulse-dev  # For PulseAudio support
  
  # Fedora/RHEL
  sudo dnf install ffmpeg-devel fftw3-devel alsa-lib-devel pulseaudio-libs-devel
  ```

## License Compatibility
All dependencies are GPL-compatible, allowing this project to be distributed under GPL v3.