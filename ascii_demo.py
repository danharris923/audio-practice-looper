#!/usr/bin/env python3
"""
ASCII Audio Practice Looper Demo
"""

import time
import os

def clear_screen():
    os.system('cls' if os.name == 'nt' else 'clear')

def format_time(seconds):
    minutes = int(seconds // 60)
    secs = seconds % 60
    return f"{minutes:02d}:{secs:04.1f}"

def draw_waveform(position, total_duration, loop_start=30, loop_end=60, width=40):
    """Draw ASCII waveform using only ASCII characters"""
    # Use only ASCII characters for waveform
    wave_pattern = ".,;:!|#+*-~.,;:!|#+*-~.,;:!|#+*-~.,;:!|"
    waveform = list((wave_pattern * (width // len(wave_pattern) + 1))[:width])
    
    # Add playback position
    pos_index = int((position / total_duration) * width)
    if 0 <= pos_index < width:
        waveform[pos_index] = "|"
    
    # Add loop markers
    loop_start_pos = int((loop_start / total_duration) * width)
    loop_end_pos = int((loop_end / total_duration) * width)
    
    if 0 <= loop_start_pos < width:
        waveform[loop_start_pos] = "["
    if 0 <= loop_end_pos < width:
        waveform[loop_end_pos] = "]"
    
    return "".join(waveform)

def show_final_state():
    """Show what the running application looks like"""
    clear_screen()
    print("=" * 65)
    print("              AUDIO PRACTICE LOOPER v1.0.0")
    print("              https://github.com/danharris923/audio-practice-looper")
    print("=" * 65)
    print()
    print("File: practice_track.mp3 (03:24.5) - 44.1kHz Stereo - BPM: 120")
    print()
    print("WAVEFORM DISPLAY:")
    print("+------------------------------------------------------------+")
    print(f"| {draw_waveform(85, 204, 30, 90, 58)} |")
    print("|                         ^playback    [loop start     ]end  |")  
    print("+------------------------------------------------------------+")
    print("Beat grid: |    |    |    |    |    |    |    |    |    |")
    print()
    print("TRANSPORT CONTROLS:")
    print("Position: 01:25.3 / 03:24.5   [PLAYING]   Loop: ON (60.0s)")
    print("[Play/Pause] [Stop] [Rewind] Volume: 85%")
    print()
    print("REAL-TIME EFFECTS:")
    print("Tempo: 110% [##########----------]  (Slightly faster)")
    print("Pitch:  +2  [----------##--------]  (+2 semitones)")  
    print("EQ: Low[######----] Mid[########--] Hi[#######---]")
    print()
    print("LOOP CONTROLS:")
    print("Active Loop: 00:30.0 -> 01:30.0 (60.0 seconds)")
    print("[Half] [Double] [-Bar] [+Bar] [Snap to Beat] [Loop Here]")
    print()
    print("EXPORT OPTIONS:")
    print("Format: [WAV] [MP3] [FLAC] [OGG]  Quality: High")
    print("Export: [Full File] [Current Loop] [Custom Range]")
    print("Effects: [Apply Tempo] [Apply Pitch] [Apply EQ]")
    print()
    print("=" * 65)
    print("FEATURES ACTIVE:")
    print("- Real-time time stretching (Rubber Band Library)")
    print("- Pitch shifting without tempo change")
    print("- Beat detection with visual grid overlay")
    print("- Professional 3-band parametric EQ")
    print("- Loop with snap-to-beat functionality") 
    print("- Multi-format audio export")
    print("- System audio capture (Windows WASAPI)")
    print("- Thread-safe audio processing")
    print("=" * 65)

def run_demo():
    """Run the demonstration"""
    stages = [
        ("Initializing Audio Practice Looper v1.0.0...", 1.5),
        ("Loading audio libraries:", 0.5),
        ("  - JUCE Framework: OK", 0.5),
        ("  - Rubber Band Library: OK", 0.5),
        ("  - aubio Beat Detection: OK", 0.5),
        ("  - FFmpeg Audio Codecs: OK", 0.5),
        ("Loading demo track: practice_track.mp3", 1),
        ("Analyzing audio:", 1),
        ("  - Sample Rate: 44.1kHz, Channels: 2", 0.5),
        ("  - Duration: 3:24.5", 0.5),
        ("  - BPM Detection: 120 BPM", 0.8),
        ("  - Beat Grid: 204 beats detected", 0.5),
        ("Audio Engine: Started", 0.5),
        ("UI Components: Loaded", 0.5),
        ("Ready for practice!", 1),
    ]
    
    print("Starting Audio Practice Looper Demo...")
    print()
    
    for message, delay in stages:
        print(f"> {message}")
        time.sleep(delay)
    
    print("\n" + "=" * 50)
    print("DEMO: Simulating typical practice session...")
    print("=" * 50)
    
    # Simulate practice actions
    actions = [
        ("User loads audio file", "File loaded successfully"),
        ("Starting playback", "Audio streaming active"),
        ("User adjusts tempo to 110%", "Real-time time stretching applied"),
        ("User shifts pitch +2 semitones", "Pitch shift applied, tempo unchanged"),
        ("User sets loop points", "Loop: 30s -> 90s (60s duration)"), 
        ("Beat detection active", "Visual beat grid overlaid on waveform"),
        ("User enables loop", "Looping active - practicing difficult section"),
        ("EQ adjustments", "3-band EQ applied in real-time"),
        ("Ready for export", "All processing can be rendered offline"),
    ]
    
    for i, (action, result) in enumerate(actions):
        time.sleep(1.5)
        print(f"{i+1}. {action}")
        print(f"   -> {result}")
    
    time.sleep(2)
    print("\nShowing application interface...")
    time.sleep(2)
    
    # Show the final interface
    show_final_state()
    
    print("\nThis demonstrates the Audio Practice Looper in action!")
    print("To build the real application, you need:")
    print("- CMake 3.21+")
    print("- Visual Studio Build Tools (Windows) or GCC/Clang (Linux)")
    print("- System audio libraries")
    print("\nThen run: python simple_build.py")

if __name__ == "__main__":
    try:
        run_demo()
    except KeyboardInterrupt:
        print("\n\nDemo interrupted.")
    finally:
        print("\nGitHub: https://github.com/danharris923/audio-practice-looper")
        print("Thanks for checking out Audio Practice Looper!")