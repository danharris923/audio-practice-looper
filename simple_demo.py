#!/usr/bin/env python3
"""
Simple Audio Practice Looper Demo
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
    """Draw ASCII waveform"""
    # Simple waveform pattern
    wave_chars = "▁▂▃▄▅▆▇█▇▆▅▄▃▂▁▂▃▄▅▆▇█▇▆▅▄▃▂▁▂▃▄▅▆▇█▇▆▅▄▃▂▁"
    waveform = (wave_chars * (width // len(wave_chars) + 1))[:width]
    
    # Add playback position
    pos_index = int((position / total_duration) * width)
    if 0 <= pos_index < width:
        waveform = waveform[:pos_index] + "|" + waveform[pos_index+1:]
    
    # Add loop markers
    loop_start_pos = int((loop_start / total_duration) * width)
    loop_end_pos = int((loop_end / total_duration) * width)
    
    if 0 <= loop_start_pos < width:
        waveform = waveform[:loop_start_pos] + "[" + waveform[loop_start_pos+1:]
    if 0 <= loop_end_pos < width:
        waveform = waveform[:loop_end_pos] + "]" + waveform[loop_end_pos+1:]
    
    return waveform

def demo_sequence():
    """Run the demo sequence"""
    # Demo state
    is_playing = False
    position = 0.0
    total_duration = 180.0  # 3 minutes
    tempo = 100
    pitch = 0
    loop_enabled = False
    bpm = 120.0
    
    actions = [
        ("Initializing Audio Practice Looper v1.0.0...", 2),
        ("Loading demo track: rock_song.mp3 (3:00)", 1),
        ("Audio analysis complete - BPM detected: 120", 1),
        ("Starting playback...", 1),
    ]
    
    # Initial setup
    print("=" * 60)
    print("         AUDIO PRACTICE LOOPER DEMO")
    print("=" * 60)
    
    for message, delay in actions:
        print(f"> {message}")
        time.sleep(delay)
    
    # Main demo loop
    for step in range(20):
        clear_screen()
        
        # Update demo state
        if step == 3:
            is_playing = True
        if step == 6:
            tempo = 120
        if step == 9:
            pitch = 2
        if step == 12:
            loop_enabled = True
        if step == 15:
            tempo = 80
        
        if is_playing:
            position += 8.0  # Simulate time passing
        
        # Draw interface
        print("=" * 60)
        print("         AUDIO PRACTICE LOOPER v1.0.0")
        print("=" * 60)
        print()
        print(f"File: rock_song.mp3 ({format_time(total_duration)}) - 44.1kHz Stereo")
        print()
        print("WAVEFORM:")
        print("+------------------------------------------+")
        print(f"| {draw_waveform(position, total_duration)} |")
        print("+------------------------------------------+")
        print(f"Position: {format_time(position)} / {format_time(total_duration)}")
        print()
        
        # Transport controls
        status = "PLAYING" if is_playing else "STOPPED"
        print(f"TRANSPORT: [{status}]  BPM: {bpm}")
        print(f"[Play] [Pause] [Stop] [Loop: {'ON' if loop_enabled else 'OFF'}]")
        print()
        
        # Effects
        tempo_bar = "#" * (tempo // 10) + "-" * (20 - tempo // 10)
        pitch_display = f"{pitch:+2d}" if pitch != 0 else " 0"
        
        print("EFFECTS:")
        print(f"Tempo: {tempo:3d}% [{tempo_bar}]")
        print(f"Pitch: {pitch_display} semitones")
        print("EQ:    Low[####--] Mid[######] Hi[#####-]")
        print()
        
        # Loop controls
        if loop_enabled:
            print("LOOP CONTROLS:")
            print(f"Loop: {format_time(30)} -> {format_time(60)} (30.0s)")
            print("[Half] [Double] [-Bar] [+Bar] [Snap to Beat]")
        else:
            print("LOOP: Click and drag on waveform to set loop points")
        print()
        
        # Status messages based on step
        if step == 3:
            print(">>> Starting playback - Audio engine active")
        elif step == 6:
            print(">>> Tempo increased to 120% - Practicing faster")
        elif step == 9:
            print(">>> Pitch shifted +2 semitones - Key change practice")
        elif step == 12:
            print(">>> Loop enabled - Repeating difficult section")
        elif step == 15:
            print(">>> Tempo reduced to 80% - Slow practice mode")
        elif step >= 17:
            print(">>> Ready for practice! All features active")
        
        print()
        print("Controls: [Space] Play/Pause [L] Loop [E] Export [Q] Quit")
        print("Real-time audio effects: Time stretching, pitch shifting, EQ")
        
        time.sleep(1.2)
    
    # Final screen
    clear_screen()
    print("=" * 60)
    print("         AUDIO PRACTICE LOOPER DEMO COMPLETE")
    print("=" * 60)
    print()
    print("Features demonstrated:")
    print("- Professional waveform display with playback position")
    print("- Real-time tempo adjustment (25% - 200%)")
    print("- Pitch shifting (±12 semitones)")
    print("- Loop controls with beat snapping")
    print("- Beat detection and BPM analysis")
    print("- 3-band parametric EQ")
    print("- Multi-format export (WAV/MP3/FLAC/OGG)")
    print()
    print("To build the real application:")
    print("1. Install CMake and Visual Studio Build Tools")
    print("2. Run: python simple_build.py")
    print("3. Or follow README.md instructions")
    print()
    print("GitHub: https://github.com/danharris923/audio-practice-looper")
    print()

if __name__ == "__main__":
    try:
        demo_sequence()
    except KeyboardInterrupt:
        print("\n\nDemo stopped. Thanks for watching!")
        print("Check out the real app at: https://github.com/danharris923/audio-practice-looper")