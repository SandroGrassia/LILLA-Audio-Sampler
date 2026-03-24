# LILLA Audio Sampler - Project Overview

## Executive Summary

**LILLA** is a professional-grade, polyphonic audio sampler based on the Teensy 4.1 microcontroller, designed and hand-assembled in Italy. The project has been evolving since 2018 and combines advanced audio processing capabilities with a user-friendly hardware interface.

The platform is **polyphonic (16 voices)**, **multitimbral**, and **multi-MIDI** compatible, allowing users to play imported audio files, self-record audio, process live streams, or use it as a MIDI looper.

**Links:**
- Website: [www.lillasampler.it](https://www.lillasampler.it/)
- Community: [www.facebook.com/Lilla.audio.sampler](https://www.facebook.com/Lilla.audio.sampler)
- Shop: [www.tindie.com/products/lillasampler](https://www.tindie.com/products/lillasampler/lilla-audio-sampler-2/)

---

## Key Features

### Operating Modes

1. **Performance Mode**: Play patches containing 1-8 sounds (layered or independent)
2. **Sampler Mode**: Record incoming audio, save to micro-SD card, and export for external use
3. **Live Sampler**: Play incoming audio stream using temporary/volatile memory as a virtual tape loop
4. **MIDI Loop**: 6-track MIDI looper for creating compositions

### Audio Capabilities

- **Audio Format Support**: 16-bit signed PCM MONO at 44.1 kHz
- **Polyphony**: 16 simultaneous voices
- **Audio Processing**:
  - ADSR envelope control
  - Pitch and length manipulation
  - Digital effects processing
  - VCF (Voltage Controlled Filter)
  - Delay effects (Stereo and Shared)
  - Live sampling with crossfading
  - Audio gain and level control
  - Peak detection and analysis

### Connectivity

| Connection | Type | Purpose |
|-----------|------|---------|
| USB-B | Power & Data | Power supply, firmware updates |
| MIDI In | 3.5mm Jack | MIDI note and control input |
| MIDI Thru | 3.5mm Jack | MIDI pass-through |
| Line In | Stereo 3.5mm Jack | 2-channel audio input (microphones/line-level) with analog gain control |
| Line Out | Stereo 3.5mm Jack | 2-channel audio output (3.3Vpp) |
| Monitor Out | Stereo 3.5mm Jack | Secondary 2-channel audio output (3.3Vpp) |
| Phones Out | 3.5mm Jack | Headphone amplified output |
| Gate In/Out | Control signals | Gate/trigger I/O |
| micro-SD Socket | Card slot | Audio file storage and export |

---

## Hardware Architecture

### Main Processor
- **MCU**: Teensy 4.1 (ARM Cortex-M7)
- **Clock Speed**: 600 MHz
- **RAM**: 1 MB total
  - RAM1 (fast): 512 KB (16 blocks × 32 KB)
  - RAM2 (slower): 512 KB (4× slower access)
- **Flash Memory**: 8 MB onboard
- **EEPROM**: 4,284 bytes (persistent storage)

### Memory Expansion
- **SPI Flash**: 64 MB (W25Q512JVFIM) - for audio file storage
- **PSRAM**: 2 × 8 MB (APS6404L-3SQR or ESP-PSRAM64H) - additional working memory

### Audio Hardware
- **Audio Adaptor**: Revision D
- **Display**: ILI9341 SPI-based TFT (240×320 pixels)
- **Digital I/O Expansion**: 6× Shift Register chips (MCP23S17)
- **Input Stage**: Microphone amplifier (AD828A) module
- **Output Stage**: Headphones amplifier module

### Compiler Configuration
- **Optimization**: Fastest (-O2 equivalent)
- **Clock**: 600 MHz

### Storage
- **Micro SD Card**: Connected via Teensy's built-in SPI interface
- **Flash Chip**: 64 MB SPI Flash for audio sample archive

---

## Software Architecture

### Core Modules

#### Audio Processing
- **AudioPlayer**: Playback engine for playback modes
- **StereoSampler / StereoLiveSampler**: Multi-voice sampler implementations
- **AudioGain / StereoGain**: Volume control and mixing
- **AudioVCF**: Voltage-controlled filter processing
- **AudioMultiplier**: Audio signal multiplication/mixing
- **AudioPeakDetector**: Peak level detection and monitoring
- **WaveLFO / WaveVibrato**: Modulation sources and effects
- **WaveSine / WavetableManager**: Waveform generation
- **StereoDelay / SharedDelay / DelayManager**: Various delay effect implementations
- **NoclickCrossmix**: Click-free crossfading between samples
- **FilterBiquadManager**: Biquad filter management for flexible EQ/filtering
- **Mixer_2x1 / SharedMixer**: Audio mixing logic
- **Router_16x3**: 16×3 audio routing matrix

#### Sample Management
- **StereoSampler**: Multi-voice playback sampler
- **PlayersManager**: Management of player instances
- **PlayersStatistics**: Performance monitoring and statistics
- **WavetableManager**: Wavetable generation and playback
- **ArchivingManager**: Audio file archival and retrieval

#### Storage & Memory
- **PsramManager**: PSRAM (external RAM) management
- **LillaSerialFlash**: SPI Flash chip interface
- **SharedVFS**: Virtual file system abstraction
- **SharedDaS**: Data storage structures

#### User Interface & Controls
- **DisplayManager**: TFT display rendering and UI updates
- **UserInterface**: Main UI navigation and state management
- **Pushbuttons**: Physical button input handling
- **Encoders**: Rotary encoder input processing
- **ShiftRegisters**: Digital I/O expansion via shift register chips
- **GraphicElements**: UI widget rendering (buttons, sliders, etc.)

#### Timing & Sequencing
- **LillaClock**: Master clock and timing
- **LoopMetronomo**: Metronome for synchronization
- **LoopLedSet / PerformanceLedSet**: LED indicator management

#### MIDI & Communications
- **MidiReader**: MIDI input parsing and handling
- **MidiOut**: MIDI output generation
- **Gate**: Gate/trigger signal control

#### Effects & Processing
- **SharedLoop**: Loop recording and playback state
- **SharedLS**: Live sampler shared functionality
- **SharedMM**: MIDI loop/metronome shared state
- **AudioFeedback**: Audio preview and feedback
- **AmpliOutMuteIn**: Output amplifier and input mute control

#### System Utilities
- **InfoMaster**: System information and status tracking
- **Functions**: Utility functions library
- **config**: Configuration management

### Build System
- **PlatformIO**: Modern embedded project management
- **Arduino Framework**: Teensy Arduino-compatible API
- **Dependencies**:
  - Adafruit GFX Library (v1.11.5+)
  - Adafruit ILI9341 (v1.5.12+)
  - Adafruit MCP23017 Library (v2.3.2+)

---

## Project Structure

### Directory Layout

```
LILLA-Audio-Sampler/
├── README.md                 # User-facing project introduction
├── LICENSE.md                # Licensing information
├── PROJECT.md                # This file
│
├── code/                      # Original LILLA firmware
│   ├── platformio.ini         # PlatformIO configuration
│   ├── include/               # Header files (.h)
│   │   ├── Audio*            # Audio processing modules
│   │   ├── *Manager.h        # Management modules
│   │   ├── UserInterface.h   # UI layer
│   │   ├── Encoders.h        # Input handling
│   │   ├── Pushbuttons.h     # Button I/O
│   │   ├── ShiftRegisters.h  # Digital expansion
│   │   ├── DisplayManager.h  # Screen output
│   │   ├── Midi*.h           # MIDI I/O
│   │   ├── SharedElements.h  # Common definitions
│   │   └── config.h          # Configuration
│   │
│   ├── src/                   # Implementation files (.cpp)
│   │   ├── main.cpp          # Firmware entry point
│   │   └── [matching .cpp files for each .h]
│   │
│   └── lib/                   # Local libraries
│       └── output_noiseshaped_pwm/
│
├── code_Lilla_2026/           # Updated/experimental firmware version
│   ├── platformio.ini
│   ├── include/
│   └── src/
│
├── doc/                       # Documentation & assets
│   └── assets/                # Images and media files
│
└── user guide/                # User documentation
```

---

## Development & Compilation

### Build Command
```bash
platformio run -e teensy41
```

### Firmware Upload
```bash
platformio run -e teensy41 --target upload
```

### Build Flags
- `-D TEENSY_OPT_FASTEST`: Fastest optimization level
- `-Wall`: All compiler warnings enabled
- Include paths configured for modular development

---

## Key Design Patterns

### Modular Architecture
- Separate header/implementation pairs for each logical component
- Shared data structures via `SharedElements.h` and related files
- Manager classes for coordinating related functionality

### Memory Efficiency
- Dual-RAM strategy (fast vs. standard)
- External PSRAM for sample storage
- SPI Flash for permanent audio archive

### Real-Time Audio Processing
- Fixed-rate audio processing at 44.1 kHz
- 16-voice polyphonic architecture
- Efficient buffer management for minimal latency

### UI Responsiveness
- Non-blocking input handling (encoders, buttons)
- Efficient display updates via DisplayManager
- LED feedback for user actions

---

## Notable Technical Features

- **Click-Free Processing**: NoclickCrossmix eliminates audible artifacts during sample transitions
- **Flexible Routing**: 16×3 audio router for complex signal flow
- **Multi-Format Support**: Handles mono audio at 44.1 kHz with 16-bit depth
- **Real-Time Archival**: Direct recording to micro-SD without compression
- **Analog Input Conditioning**: Integrated microphone preamplifier with gain control
- **MIDI Implementation**: Full MIDI in/out/thru with 6-track looping capability

---

## Project Evolution

The LILLA project began in 2018 and has gone through multiple hardware revisions and firmware improvements. The codebase reflects continuous refinement, with specialized modules developed for each functional area (audio processing, storage, UI, MIDI communication).

Two main firmware versions are maintained:
1. **Original `code/`**: Main stable firmware
2. **`code_Lilla_2026/`**: Updated/experimental version

---

## Author & Contact

- **Author**: Sandro Grassia
- **Email**: info@lillasampler.it
- **Website**: www.lillasampler.it
