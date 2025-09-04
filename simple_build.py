#!/usr/bin/env python3
"""
Simple Audio Practice Looper Build Script
"""

import os
import sys
import subprocess
import platform
import shutil
from pathlib import Path

def run_command(cmd, cwd=None):
    """Run a command and return success status"""
    print(f"Running: {' '.join(cmd) if isinstance(cmd, list) else cmd}")
    try:
        result = subprocess.run(cmd, cwd=cwd, check=True, shell=True)
        return True
    except subprocess.CalledProcessError as e:
        print(f"Command failed with error: {e}")
        return False

def main():
    print("Audio Practice Looper - Simple Build Script")
    print("=" * 45)
    
    # Check Python version
    if sys.version_info < (3, 7):
        print("ERROR: Python 3.7+ required")
        return False
    print(f"Python {sys.version.split()[0]} OK")
    
    # Detect platform
    system = platform.system().lower()
    print(f"Platform: {system}")
    
    # Check for CMake
    try:
        result = subprocess.run(["cmake", "--version"], capture_output=True, text=True)
        print("CMake found:", result.stdout.split('\n')[0])
        cmake_available = True
    except FileNotFoundError:
        print("CMake not found")
        cmake_available = False
    
    # Check for compiler
    compilers = ["g++", "clang++", "cl"]
    compiler_found = False
    for compiler in compilers:
        try:
            subprocess.run([compiler, "--version"], capture_output=True, check=True)
            print(f"Compiler found: {compiler}")
            compiler_found = True
            break
        except (FileNotFoundError, subprocess.CalledProcessError):
            continue
    
    if not compiler_found:
        print("No C++ compiler found")
    
    # Show what we would do
    print("\nBuild process would:")
    print("1. Install system dependencies")
    if system == "linux":
        print("   - sudo apt install build-essential cmake libasound2-dev libpulse-dev")
        print("   - sudo apt install libx11-dev libxrandr-dev libxinerama-dev")
    elif system == "windows":
        print("   - Download and install CMake")
        print("   - Check for Visual Studio Build Tools")
    elif system == "darwin":
        print("   - brew install cmake pkg-config")
    
    print("2. Create build directory")
    print("3. Configure with CMake")
    print("4. Compile the application")
    print("5. Run tests")
    
    # Try to actually build if tools are available
    if cmake_available and compiler_found:
        print("\nAttempting to build...")
        
        project_root = Path(__file__).parent
        build_dir = project_root / "build"
        
        # Create build directory
        if build_dir.exists():
            shutil.rmtree(build_dir)
        build_dir.mkdir()
        print(f"Created build directory: {build_dir}")
        
        # Configure
        print("Configuring with CMake...")
        if run_command(["cmake", "..", "-DCMAKE_BUILD_TYPE=Release"], cwd=build_dir):
            print("CMake configuration successful!")
            
            # Build
            print("Building project...")
            if run_command(["cmake", "--build", ".", "--config", "Release"], cwd=build_dir):
                print("BUILD SUCCESSFUL!")
                
                # Look for executable
                possible_exes = [
                    build_dir / "bin" / "AudioPracticeLooper",
                    build_dir / "bin" / "Release" / "AudioPracticeLooper.exe",
                    build_dir / "src" / "AudioPracticeLooper", 
                    build_dir / "src" / "Release" / "AudioPracticeLooper.exe"
                ]
                
                for exe in possible_exes:
                    if exe.exists():
                        print(f"Executable found: {exe}")
                        print("You can now run the Audio Practice Looper!")
                        return True
                        
                print("Build completed but executable not found in expected locations")
                return True
            else:
                print("Build failed")
                return False
        else:
            print("CMake configuration failed")
            return False
    else:
        print("\nMissing build tools. Please install:")
        if not cmake_available:
            print("- CMake 3.21+ (https://cmake.org/download/)")
        if not compiler_found:
            if system == "windows":
                print("- Visual Studio Build Tools or Visual Studio Community")
            elif system == "linux":
                print("- build-essential package (sudo apt install build-essential)")
            elif system == "darwin":
                print("- Xcode Command Line Tools (xcode-select --install)")
        return False

if __name__ == "__main__":
    success = main()
    if not success:
        print("\nBuild failed or tools missing. See requirements above.")
        sys.exit(1)
    else:
        print("\nBuild script completed successfully!")