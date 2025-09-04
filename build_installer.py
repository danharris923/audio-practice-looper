#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Audio Practice Looper - Automated Build and Install Script
Handles dependency installation and compilation across platforms
"""

import os
import sys
import subprocess
import platform
import shutil
import urllib.request
import zipfile
import tarfile
from pathlib import Path

# Set UTF-8 encoding for Windows
if platform.system() == "Windows":
    import codecs
    sys.stdout = codecs.getwriter('utf-8')(sys.stdout.buffer, 'strict')
    sys.stderr = codecs.getwriter('utf-8')(sys.stderr.buffer, 'strict')

class BuildInstaller:
    def __init__(self):
        self.system = platform.system().lower()
        self.project_root = Path(__file__).parent
        self.build_dir = self.project_root / "build"
        
        # Version requirements
        self.cmake_min_version = "3.21"
        self.required_packages = {
            "linux": [
                "build-essential", "cmake", "git", "pkg-config",
                "libasound2-dev", "libpulse-dev", "libpulse-simple0",
                "libx11-dev", "libxrandr-dev", "libxinerama-dev", "libxcursor-dev",
                "libfreetype6-dev", "libgl1-mesa-dev"
            ],
            "windows": ["cmake", "git"],
            "darwin": ["cmake", "git"]
        }
        
    def log(self, message, level="INFO"):
        """Log messages with colors"""
        colors = {
            "INFO": "\033[94m",    # Blue
            "SUCCESS": "\033[92m", # Green
            "WARNING": "\033[93m", # Yellow
            "ERROR": "\033[91m",   # Red
            "RESET": "\033[0m"
        }
        print(f"{colors.get(level, '')}{level}: {message}{colors['RESET']}")
        
    def run_command(self, cmd, cwd=None, check=True):
        """Run shell command with error handling"""
        try:
            self.log(f"Running: {' '.join(cmd) if isinstance(cmd, list) else cmd}")
            result = subprocess.run(
                cmd, 
                shell=isinstance(cmd, str),
                cwd=cwd,
                check=check,
                capture_output=True,
                text=True
            )
            if result.stdout:
                print(result.stdout)
            return result
        except subprocess.CalledProcessError as e:
            self.log(f"Command failed: {e}", "ERROR")
            if e.stderr:
                print(e.stderr)
            raise
            
    def check_python_version(self):
        """Ensure Python 3.7+"""
        if sys.version_info < (3, 7):
            self.log("Python 3.7+ required", "ERROR")
            sys.exit(1)
        self.log(f"Python {sys.version.split()[0]} ✓", "SUCCESS")
        
    def install_cmake_windows(self):
        """Download and install CMake on Windows"""
        self.log("Installing CMake on Windows...")
        
        cmake_url = "https://github.com/Kitware/CMake/releases/download/v3.27.7/cmake-3.27.7-windows-x86_64.zip"
        cmake_zip = self.project_root / "cmake.zip"
        cmake_dir = self.project_root / "cmake"
        
        # Download CMake
        self.log("Downloading CMake...")
        urllib.request.urlretrieve(cmake_url, cmake_zip)
        
        # Extract
        with zipfile.ZipFile(cmake_zip, 'r') as zip_ref:
            zip_ref.extractall(cmake_dir)
            
        # Find cmake executable
        for root, dirs, files in os.walk(cmake_dir):
            if "cmake.exe" in files:
                cmake_path = Path(root) / "cmake.exe"
                # Add to PATH for this session
                os.environ["PATH"] = f"{cmake_path.parent};{os.environ.get('PATH', '')}"
                self.log(f"CMake installed at {cmake_path}", "SUCCESS")
                break
                
        # Cleanup
        cmake_zip.unlink()
        
    def install_dependencies_linux(self):
        """Install dependencies on Linux using apt"""
        self.log("Installing dependencies on Linux...")
        
        # Update package list
        self.run_command(["sudo", "apt", "update"])
        
        # Install packages
        packages = self.required_packages["linux"]
        self.run_command(["sudo", "apt", "install", "-y"] + packages)
        self.log("Linux dependencies installed", "SUCCESS")
        
    def install_dependencies_windows(self):
        """Install dependencies on Windows"""
        self.log("Setting up Windows build environment...")
        
        # Check for Visual Studio Build Tools
        vs_paths = [
            r"C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvarsall.bat",
            r"C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat",
            r"C:\Program Files\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat",
            r"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat"
        ]
        
        vs_found = False
        for vs_path in vs_paths:
            if os.path.exists(vs_path):
                self.log(f"Found Visual Studio at {vs_path}", "SUCCESS")
                vs_found = True
                break
                
        if not vs_found:
            self.log("Visual Studio Build Tools not found. Please install Visual Studio 2019+ or Build Tools", "WARNING")
            self.log("Download from: https://visualstudio.microsoft.com/downloads/", "INFO")
            
        # Install CMake if not available
        try:
            self.run_command(["cmake", "--version"])
            self.log("CMake already installed", "SUCCESS")
        except (subprocess.CalledProcessError, FileNotFoundError):
            self.install_cmake_windows()
            
    def install_dependencies_mac(self):
        """Install dependencies on macOS using Homebrew"""
        self.log("Installing dependencies on macOS...")
        
        # Check for Homebrew
        try:
            self.run_command(["brew", "--version"])
        except (subprocess.CalledProcessError, FileNotFoundError):
            self.log("Homebrew not found. Installing Homebrew...", "WARNING")
            install_homebrew = '/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"'
            self.run_command(install_homebrew)
            
        # Install packages
        packages = ["cmake", "git", "pkg-config"]
        self.run_command(["brew", "install"] + packages)
        self.log("macOS dependencies installed", "SUCCESS")
        
    def install_dependencies(self):
        """Install platform-specific dependencies"""
        self.log(f"Detected platform: {self.system}")
        
        if self.system == "linux":
            self.install_dependencies_linux()
        elif self.system == "windows":
            self.install_dependencies_windows()
        elif self.system == "darwin":
            self.install_dependencies_mac()
        else:
            self.log(f"Unsupported platform: {self.system}", "ERROR")
            sys.exit(1)
            
    def setup_build_directory(self):
        """Create and setup build directory"""
        self.log("Setting up build directory...")
        
        if self.build_dir.exists():
            shutil.rmtree(self.build_dir)
            
        self.build_dir.mkdir(parents=True)
        self.log(f"Build directory created: {self.build_dir}", "SUCCESS")
        
    def configure_cmake(self):
        """Configure project with CMake"""
        self.log("Configuring with CMake...")
        
        cmake_args = [
            "cmake", "..",
            "-DCMAKE_BUILD_TYPE=Release",
            "-DCMAKE_CXX_STANDARD=20",
        ]
        
        # Platform-specific configuration
        if self.system == "windows":
            cmake_args.extend([
                "-A", "x64",  # 64-bit architecture
                "-DCMAKE_GENERATOR_PLATFORM=x64"
            ])
            
        self.run_command(cmake_args, cwd=self.build_dir)
        self.log("CMake configuration completed", "SUCCESS")
        
    def build_project(self):
        """Build the project"""
        self.log("Building Audio Practice Looper...")
        
        build_args = [
            "cmake", "--build", ".",
            "--config", "Release",
            "--parallel", str(os.cpu_count() or 4)
        ]
        
        self.run_command(build_args, cwd=self.build_dir)
        self.log("Build completed successfully! 🎉", "SUCCESS")
        
    def run_tests(self):
        """Run unit tests if available"""
        self.log("Running tests...")
        
        try:
            self.run_command(["ctest", "--verbose"], cwd=self.build_dir)
            self.log("All tests passed! ✅", "SUCCESS")
        except subprocess.CalledProcessError:
            self.log("Some tests failed, but build is complete", "WARNING")
            
    def find_executable(self):
        """Find the built executable"""
        possible_paths = [
            self.build_dir / "bin" / "AudioPracticeLooper",
            self.build_dir / "bin" / "Release" / "AudioPracticeLooper.exe",
            self.build_dir / "src" / "AudioPracticeLooper",
            self.build_dir / "src" / "Release" / "AudioPracticeLooper.exe"
        ]
        
        for exe_path in possible_paths:
            if exe_path.exists():
                return exe_path
                
        return None
        
    def create_launcher_script(self):
        """Create platform-appropriate launcher script"""
        exe_path = self.find_executable()
        
        if not exe_path:
            self.log("Could not find executable", "WARNING")
            return
            
        if self.system == "windows":
            launcher_path = self.project_root / "run_audio_looper.bat"
            with open(launcher_path, 'w') as f:
                f.write(f'@echo off\ncd /d "{self.build_dir}"\n"{exe_path.relative_to(self.build_dir)}"\npause\n')
        else:
            launcher_path = self.project_root / "run_audio_looper.sh"
            with open(launcher_path, 'w') as f:
                f.write(f'#!/bin/bash\ncd "{self.build_dir}"\n"./{exe_path.relative_to(self.build_dir)}"\n')
            os.chmod(launcher_path, 0o755)
            
        self.log(f"Launcher created: {launcher_path}", "SUCCESS")
        return launcher_path
        
    def run_application(self):
        """Attempt to run the built application"""
        exe_path = self.find_executable()
        
        if not exe_path:
            self.log("Executable not found", "ERROR")
            return False
            
        self.log(f"Starting Audio Practice Looper: {exe_path}")
        
        try:
            # Run in background so script can continue
            if self.system == "windows":
                subprocess.Popen([str(exe_path)], cwd=exe_path.parent)
            else:
                subprocess.Popen([str(exe_path)], cwd=exe_path.parent)
                
            self.log("🎵 Audio Practice Looper started! 🎵", "SUCCESS")
            return True
            
        except Exception as e:
            self.log(f"Failed to start application: {e}", "ERROR")
            return False
            
    def print_summary(self):
        """Print build summary and usage instructions"""
        print("\n" + "="*60)
        print("🎵 AUDIO PRACTICE LOOPER BUILD COMPLETE! 🎵")
        print("="*60)
        
        exe_path = self.find_executable()
        if exe_path:
            print(f"📍 Executable location: {exe_path}")
            
        launcher_path = self.project_root / ("run_audio_looper.bat" if self.system == "windows" else "run_audio_looper.sh")
        if launcher_path.exists():
            print(f"🚀 Quick launcher: {launcher_path}")
            
        print("\n📖 Usage Instructions:")
        print("1. Load an audio file (MP3, WAV, FLAC)")
        print("2. Use transport controls to play/pause")
        print("3. Adjust tempo (25-200%) and pitch (±12 semitones)")
        print("4. Set loop points by dragging on waveform")
        print("5. Use loop controls for snap-to-beat and quick adjustments")
        print("6. Export processed audio when ready")
        
        print("\n🔧 Features Available:")
        print("- Real-time time stretching and pitch shifting")
        print("- Beat detection with visual beat grid")
        print("- Professional 3-band EQ")
        print("- System audio capture (Windows/Linux)")
        print("- Multi-format export (WAV/MP3/FLAC/OGG)")
        print("- Interactive waveform display with zoom/scroll")
        
        print("\n" + "="*60)
        
    def build_and_install(self):
        """Main build and install process"""
        try:
            print("Audio Practice Looper - Build & Install Script")
            print("=" * 50)
            
            self.check_python_version()
            self.install_dependencies()
            self.setup_build_directory()
            self.configure_cmake()
            self.build_project()
            self.run_tests()
            
            launcher_path = self.create_launcher_script()
            self.print_summary()
            
            # Ask user if they want to run the app
            if launcher_path:
                try:
                    response = input("\n🚀 Would you like to start Audio Practice Looper now? (y/N): ").strip().lower()
                    if response in ['y', 'yes']:
                        if not self.run_application():
                            self.log(f"You can manually run: {launcher_path}", "INFO")
                except KeyboardInterrupt:
                    print("\nBuild completed successfully!")
                    
        except KeyboardInterrupt:
            self.log("Build cancelled by user", "WARNING")
            sys.exit(1)
        except Exception as e:
            self.log(f"Build failed: {e}", "ERROR")
            sys.exit(1)

def main():
    """Entry point"""
    if len(sys.argv) > 1 and sys.argv[1] in ['-h', '--help']:
        print(__doc__)
        print("\nUsage:")
        print("  python build_installer.py          # Full build and install")
        print("  python build_installer.py --help   # Show this help")
        return
        
    installer = BuildInstaller()
    installer.build_and_install()

if __name__ == "__main__":
    main()