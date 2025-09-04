#!/usr/bin/env python3
"""
Audio Practice Looper - Demo Application
Simulates the real application interface and functionality
"""

import time
import os
import threading
from pathlib import Path

class AudioPracticeLooperDemo:
    def __init__(self):
        self.is_playing = False
        self.current_position = 0.0
        self.total_duration = 180.0  # 3 minutes demo track
        self.tempo = 100  # 100% = normal speed
        self.pitch = 0    # 0 = no pitch shift
        self.loop_enabled = False
        self.loop_start = 30.0
        self.loop_end = 60.0
        self.current_bpm = 120.0
        self.volume = 0.8
        
    def clear_screen(self):
        os.system('cls' if os.name == 'nt' else 'clear')
        
    def format_time(self, seconds):
        """Format time as MM:SS.s"""
        minutes = int(seconds // 60)
        secs = seconds % 60
        return f"{minutes:02d}:{secs:04.1f}"
        
    def draw_waveform(self, width=50):
        """Draw a simple ASCII waveform with playback position"""
        waveform = [
            "▁▂▃▅▇█▇▅▃▂▁▂▃▅▇██▇▅▃▂▁▂▃▄▅▆▇▆▅▄▃▂▁▂▃▄▅▆▇█▇▆▅▄▃▂▁▂▃▄▅▆",
            "▂▃▄▆██▆▄▃▂▃▄▆███▆▄▃▂▃▄▅▆▇▆▅▄▃▂▃▄▅▆▇█▇▆▅▄▃▂▃▄▅▆▇▇▆▅▄▃▂",
            "▃▄▅▇██▇▅▄▃▄▅▇███▇▅▄▃▄▅▆▇▆▅▄▃▄▅▆▇▇▆▅▄▃▄▅▆▇██▇▆▅▄▃▄▅▆▇▆▅"
        ]
        
        # Calculate playback position on waveform
        progress = self.current_position / self.total_duration
        playback_pos = int(progress * width)
        
        # Draw waveform with playback indicator
        result = []
        for wave_line in waveform:
            line = wave_line[:width].ljust(width, "▁")
            # Insert playback position marker
            if 0 <= playback_pos < len(line):
                line = line[:playback_pos] + "|" + line[playback_pos+1:]
            result.append(line)
            
        # Add loop markers
        if self.loop_enabled:
            loop_start_pos = int((self.loop_start / self.total_duration) * width)
            loop_end_pos = int((self.loop_end / self.total_duration) * width)
            
            for i, line in enumerate(result):
                line_chars = list(line)
                if 0 <= loop_start_pos < len(line_chars):
                    line_chars[loop_start_pos] = "["
                if 0 <= loop_end_pos < len(line_chars):
                    line_chars[loop_end_pos] = "]"
                result[i] = "".join(line_chars)
                
        return result
        
    def draw_controls(self):
        """Draw transport and effect controls"""
        # Transport controls
        play_symbol = "⏸️" if self.is_playing else "▶️"
        loop_symbol = "🔄" if self.loop_enabled else "○"
        
        controls = f"""
┌─────────────── TRANSPORT CONTROLS ───────────────┐
│ {play_symbol} Play/Pause    ⏹️ Stop    {loop_symbol} Loop       │
│ Position: {self.format_time(self.current_position)} / {self.format_time(self.total_duration)}              │
│ BPM: {self.current_bpm:5.1f}    Volume: {self.volume*100:3.0f}%           │
└───────────────────────────────────────────────────┘

┌─────────────── EFFECT CONTROLS ──────────────────┐
│ Tempo:  {self.tempo:3d}% {'█' * (self.tempo//10)}{'░' * (20-self.tempo//10)} │
│ Pitch: {self.pitch:+3d}♪  {'█' * (10+self.pitch)}{'░' * (10-self.pitch)}     │
│ EQ: Low [████░░░] Mid [██████░] Hi [███████]    │
└───────────────────────────────────────────────────┘
"""
        return controls
        
    def draw_loop_controls(self):
        """Draw loop manipulation controls"""
        loop_info = ""
        if self.loop_enabled:
            loop_duration = self.loop_end - self.loop_start
            loop_info = f"Loop: {self.format_time(self.loop_start)} → {self.format_time(self.loop_end)} ({self.format_time(loop_duration)})"
        else:
            loop_info = "Loop: Disabled"
            
        controls = f"""
┌─────────────── LOOP CONTROLS ────────────────────┐
│ {loop_info}                     │
│ [×½] Half   [×2] Double   [-Bar] [+Bar]         │
│ [◄Beat] Snap Start  [Beat►] Snap End  [◄►Beat]  │
│ [Loop Here] Create loop at current position     │
└───────────────────────────────────────────────────┘
"""
        return controls
        
    def draw_export_options(self):
        """Draw export options"""
        return """
┌─────────────── EXPORT OPTIONS ───────────────────┐
│ Format: [WAV] [MP3] [FLAC] [OGG]                 │
│ Range:  [Full File] [Current Loop] [Custom]      │
│ Quality: High    Effects: [Tempo] [Pitch] [EQ]   │
│ Status: Ready to export                          │
└───────────────────────────────────────────────────┘
"""

    def draw_interface(self):
        """Draw the complete application interface"""
        self.clear_screen()
        
        header = """
╔════════════════════════════════════════════════════╗
║           🎵 AUDIO PRACTICE LOOPER v1.0.0           ║
║              Professional Practice Tool              ║
╚════════════════════════════════════════════════════╝

File: demo_track.mp3 (3:00) - 44.1kHz Stereo
"""
        
        # Draw waveform
        waveform_lines = self.draw_waveform(50)
        waveform_display = """
┌─────────────── WAVEFORM DISPLAY ─────────────────┐
│ """ + waveform_lines[0] + """ │
│ """ + waveform_lines[1] + """ │  
│ """ + waveform_lines[2] + """ │
│ Beat Grid: ♦   ♦   ♦   ♦   ♦   ♦   ♦   ♦   ♦   ♦ │
└───────────────────────────────────────────────────┘
"""

        # Combine all interface elements
        interface = (
            header + 
            waveform_display +
            self.draw_controls() +
            self.draw_loop_controls() +
            self.draw_export_options()
        )
        
        # Add instructions
        instructions = """
┌─────────────────── INSTRUCTIONS ──────────────────┐
│ [SPACE] Play/Pause    [L] Toggle Loop             │
│ [←/→] Seek    [↑/↓] Tempo    [+/-] Pitch          │
│ [1-5] Set Loop Start    [6-0] Set Loop End        │
│ [E] Export    [Q] Quit    [R] Reset Effects       │
└────────────────────────────────────────────────────┘
"""
        
        print(interface + instructions)
        
        # Show current status
        status = "PLAYING" if self.is_playing else "PAUSED"
        effects_status = f"Tempo: {self.tempo}%  Pitch: {self.pitch:+d}♪"
        if self.loop_enabled:
            effects_status += f"  Loop: ON ({self.format_time(self.loop_end - self.loop_start)})"
        
        print(f"\nStatus: {status}  |  {effects_status}")
        print("Press Ctrl+C to stop demo")
        
    def simulate_playback(self):
        """Simulate audio playback"""
        while True:
            if self.is_playing:
                # Advance position based on tempo
                time_increment = 0.1 * (self.tempo / 100.0)
                self.current_position += time_increment
                
                # Handle looping
                if self.loop_enabled:
                    if self.current_position >= self.loop_end:
                        self.current_position = self.loop_start
                else:
                    if self.current_position >= self.total_duration:
                        self.current_position = 0.0
                        self.is_playing = False
                        
            time.sleep(0.1)
            
    def handle_input(self):
        """Handle keyboard input (simplified)"""
        import sys
        import select
        
        while True:
            try:
                # This is a simplified input handler
                # In a real GUI application, this would be event-driven
                time.sleep(0.5)  # Update interface every 500ms
                self.draw_interface()
            except KeyboardInterrupt:
                print("\n\nDemo stopped. Thanks for trying Audio Practice Looper!")
                break
                
    def run_demo(self):
        """Run the demo application"""
        print("Starting Audio Practice Looper Demo...")
        time.sleep(1)
        
        # Start playback simulation in background
        playback_thread = threading.Thread(target=self.simulate_playback, daemon=True)
        playback_thread.start()
        
        # Simulate some user actions
        actions = [
            (2, "Loading audio file...", lambda: setattr(self, 'current_position', 0)),
            (1, "Starting playback", lambda: setattr(self, 'is_playing', True)),
            (3, "Adjusting tempo to 120%", lambda: setattr(self, 'tempo', 120)),
            (2, "Shifting pitch up 2 semitones", lambda: setattr(self, 'pitch', 2)),
            (3, "Enabling loop", lambda: setattr(self, 'loop_enabled', True)),
            (4, "Snapping loop to beats", lambda: None),
            (2, "Adjusting tempo to 80%", lambda: setattr(self, 'tempo', 80)),
            (3, "Ready for practice!", lambda: None)
        ]
        
        try:
            for delay, message, action in actions:
                time.sleep(delay)
                action()
                self.draw_interface()
                print(f"\n> {message}")
                
            # Continue running with user controls
            self.handle_input()
            
        except KeyboardInterrupt:
            print("\n\nDemo stopped. Thanks for trying Audio Practice Looper!")

def main():
    """Run the demo application"""
    demo = AudioPracticeLooperDemo()
    demo.run_demo()

if __name__ == "__main__":
    main()