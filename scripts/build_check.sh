#!/bin/bash

# Build verification script for Audio Practice Looper
# This script checks dependencies and attempts to build the project

set -e  # Exit on any error

echo "🎵 Audio Practice Looper - Build Check Script"
echo "=============================================="

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}✅ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

print_error() {
    echo -e "${RED}❌ $1${NC}"
}

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    print_error "CMakeLists.txt not found. Please run this script from the project root directory."
    exit 1
fi

print_status "Found project root directory"

# Check system dependencies
echo ""
echo "Checking system dependencies..."

# Check CMake
if command -v cmake &> /dev/null; then
    CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
    print_status "CMake found: $CMAKE_VERSION"
    
    # Check if version is sufficient (3.21+)
    CMAKE_MAJOR=$(echo $CMAKE_VERSION | cut -d'.' -f1)
    CMAKE_MINOR=$(echo $CMAKE_VERSION | cut -d'.' -f2)
    
    if [ "$CMAKE_MAJOR" -gt 3 ] || ([ "$CMAKE_MAJOR" -eq 3 ] && [ "$CMAKE_MINOR" -ge 21 ]); then
        print_status "CMake version is sufficient (3.21+ required)"
    else
        print_warning "CMake version $CMAKE_VERSION may be too old. 3.21+ recommended."
    fi
else
    print_error "CMake not found. Please install CMake 3.21 or later."
    exit 1
fi

# Check C++ compiler
if command -v g++ &> /dev/null; then
    GCC_VERSION=$(g++ --version | head -n1)
    print_status "GCC found: $GCC_VERSION"
elif command -v clang++ &> /dev/null; then
    CLANG_VERSION=$(clang++ --version | head -n1)
    print_status "Clang found: $CLANG_VERSION"
elif command -v cl &> /dev/null; then
    print_status "MSVC found"
else
    print_error "No C++ compiler found. Please install GCC, Clang, or MSVC."
    exit 1
fi

# Check Git
if command -v git &> /dev/null; then
    GIT_VERSION=$(git --version)
    print_status "Git found: $GIT_VERSION"
else
    print_error "Git not found. Git is required for dependency management."
    exit 1
fi

# Platform-specific dependency checks
echo ""
echo "Checking platform-specific dependencies..."

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    print_status "Detected Linux platform"
    
    # Check for audio libraries
    if pkg-config --exists alsa; then
        print_status "ALSA found"
    else
        print_warning "ALSA not found. Install libasound2-dev (Ubuntu/Debian) or alsa-lib (Arch)"
    fi
    
    if pkg-config --exists libpulse; then
        print_status "PulseAudio found"
    else
        print_warning "PulseAudio not found. Install libpulse-dev (Ubuntu/Debian) or pulseaudio (Arch)"
    fi
    
    # Check for X11 libraries
    if pkg-config --exists x11; then
        print_status "X11 found"
    else
        print_warning "X11 not found. Install libx11-dev and related X11 packages"
    fi

elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
    print_status "Detected Windows platform"
    print_status "Windows audio libraries (WASAPI) should be available by default"

elif [[ "$OSTYPE" == "darwin"* ]]; then
    print_status "Detected macOS platform"
    print_status "CoreAudio should be available by default"
    
else
    print_warning "Unknown platform: $OSTYPE"
fi

# Try to create build directory
echo ""
echo "Setting up build environment..."

BUILD_DIR="build"
if [ -d "$BUILD_DIR" ]; then
    print_warning "Build directory already exists"
else
    mkdir "$BUILD_DIR"
    print_status "Created build directory"
fi

cd "$BUILD_DIR"

# Try CMake configuration
echo ""
echo "Attempting CMake configuration..."

if cmake .. -DCMAKE_BUILD_TYPE=Release; then
    print_status "CMake configuration successful"
    
    # Try to build
    echo ""
    echo "Attempting to build project..."
    
    if cmake --build . --config Release; then
        print_status "Build successful! 🎉"
        echo ""
        echo "Build artifacts should be available in:"
        echo "  - Executable: bin/AudioPracticeLooper (Linux) or bin/Release/AudioPracticeLooper.exe (Windows)"
        echo ""
        echo "To run the application:"
        echo "  cd build"
        echo "  ./bin/AudioPracticeLooper  # Linux"
        echo "  ./bin/Release/AudioPracticeLooper.exe  # Windows"
        
    else
        print_error "Build failed. Check the output above for errors."
        echo ""
        echo "Common issues:"
        echo "  - Missing system dependencies (see warnings above)"
        echo "  - Network issues downloading dependencies"
        echo "  - Compiler compatibility issues"
        exit 1
    fi
    
else
    print_error "CMake configuration failed."
    echo ""
    echo "Common issues:"
    echo "  - CMake version too old (3.21+ required)"
    echo "  - Missing system dependencies"
    echo "  - Network issues accessing external repositories"
    exit 1
fi

# Try to run tests if available
if [ -f "tests/AudioPracticeLooperTests" ] || [ -f "tests/Release/AudioPracticeLooperTests.exe" ]; then
    echo ""
    echo "Running tests..."
    
    if ctest --verbose; then
        print_status "All tests passed! ✅"
    else
        print_warning "Some tests failed. Check output above."
    fi
fi

echo ""
print_status "Build check completed successfully! 🚀"
echo ""
echo "Next steps:"
echo "  1. Load an audio file (MP3, WAV, or FLAC)"
echo "  2. Use transport controls to play/pause"
echo "  3. Adjust tempo and pitch in real-time"
echo "  4. Set loop points and practice sections"
echo "  5. Export processed audio when ready"