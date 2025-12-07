# SteamVR Fingers - Valve Index Finger Tracking for Godot 4.3

A complete GDExtension that provides individual finger curl tracking for Valve Index controllers in Godot 4.3 using the OpenVR/SteamVR API.

## Features

- **Full finger tracking** for all 10 fingers (5 per hand)
- Individual curl values from 0.0 (open) to 1.0 (closed) for:
  - Thumb, Index, Middle, Ring, and Pinky on both hands
- **Real-time updates** via `_process()`
- **Automatic initialization** in `_ready()`
- **Graceful error handling** - returns 0.0 values when SteamVR is unavailable
- **Cross-platform ready** (Windows x64 included, Linux support ready)

## Project Structure

```
steamvr-fingers-extension/
├── godot-cpp/              # Godot 4.3 bindings (submodule)
├── openvr/                 # OpenVR SDK (submodule)
├── src/
│   ├── steamvr_fingers.h
│   ├── steamvr_fingers.cpp
│   ├── register_types.h
│   └── register_types.cpp
├── demo/                   # Demo Godot project
│   ├── addons/
│   │   └── steamvr_fingers/
│   │       ├── actions/
│   │       │   ├── action_manifest.json
│   │       │   └── knuckles_bindings.json
│   │       ├── bin/        # Compiled extension + DLLs go here
│   │       └── steamvr_fingers.gdextension
│   ├── main.gd
│   ├── main.tscn
│   └── project.godot
├── SConstruct              # Build configuration
├── .gitignore
└── README.md
```

## Prerequisites

### Required Software

1. **Godot 4.3** - Download from [godotengine.org](https://godotengine.org/download)
2. **Python 3.6+** - For SCons build system
3. **SCons** - Install via `pip install scons`
4. **C++ Compiler**:
   - **Windows**: Visual Studio 2019 or later (with C++ desktop development)
   - **Linux**: GCC 9+ or Clang 10+
5. **Git** - For cloning submodules
6. **SteamVR** - Installed and configured with Valve Index controllers

### Hardware Requirements

- **Valve Index Controllers** (Knuckles)
- **VR Headset** compatible with SteamVR
- **Windows 10/11** (primary support) or **Linux**

## Building the Extension

### 1. Clone the Repository

```bash
git clone --recursive https://github.com/yourusername/steamvr-fingers-extension.git
cd steamvr-fingers-extension
```

If you already cloned without `--recursive`, initialize submodules:

```bash
git submodule update --init --recursive
```

### 2. Build godot-cpp

**Windows (Visual Studio Command Prompt):**

```bash
# Debug build
cd godot-cpp
scons platform=windows target=template_debug

# Release build
scons platform=windows target=template_release
cd ..
```

**Linux:**

```bash
# Debug build
cd godot-cpp
scons platform=linux target=template_debug

# Release build
scons platform=linux target=template_release
cd ..
```

### 3. Build the Extension

**Windows:**

```bash
# Debug build
scons platform=windows target=template_debug

# Release build
scons platform=windows target=template_release
```

**Linux:**

```bash
# Debug build
scons platform=linux target=template_debug

# Release build
scons platform=linux target=template_release
```

The build process will automatically:
- Compile the extension
- Copy `openvr_api.dll` (Windows) or `libopenvr_api.so` (Linux) to `demo/addons/steamvr_fingers/bin/`
- Place the compiled extension in the bin folder

### 4. Verify Build Output

Check that `demo/addons/steamvr_fingers/bin/` contains:

**Windows:**
- `libsteamvr_fingers.windows.template_debug.x86_64.dll`
- `libsteamvr_fingers.windows.template_release.x86_64.dll`
- `openvr_api.dll`

**Linux:**
- `libsteamvr_fingers.linux.template_debug.x86_64.so`
- `libsteamvr_fingers.linux.template_release.x86_64.so`
- `libopenvr_api.so`

## Running the Demo

1. Open Godot 4.3
2. Click "Import"
3. Navigate to `demo/project.godot` and open it
4. Ensure SteamVR is running with Index controllers connected
5. Press F5 to run the demo

The demo displays real-time finger curl values for all 10 fingers with color coding:
- **Green**: Open (0.0 - 0.3)
- **Yellow**: Partially closed (0.3 - 0.7)
- **Red**: Closed (0.7 - 1.0)

## Usage in Your Project

### 1. Copy the Extension

Copy the entire `demo/addons/steamvr_fingers/` folder to your Godot project's `addons/` directory.

### 2. Enable the Extension

Godot should automatically detect the extension. If not, restart the editor.

### 3. Use in GDScript

```gdscript
extends Node

var finger_tracker: SteamVRFingers

func _ready():
    # Create and add the finger tracker
    finger_tracker = SteamVRFingers.new()
    add_child(finger_tracker)

    # Check if initialization succeeded
    if finger_tracker.is_steamvr_initialized():
        print("SteamVR finger tracking active!")
    else:
        print("SteamVR not available")

func _process(delta):
    # Get individual finger curl values (0.0 = open, 1.0 = closed)
    var left_index = finger_tracker.get_left_index_curl()
    var right_thumb = finger_tracker.get_right_thumb_curl()

    # Use the values for game logic
    if left_index > 0.8:
        print("Left index finger is fully curled!")
```

## API Reference

### SteamVRFingers Class

Inherits: `Node`

#### Methods

**Initialization:**
- `bool initialize_steamvr()` - Manually initialize SteamVR (called automatically in `_ready()`)
- `void shutdown_steamvr()` - Shutdown SteamVR (called automatically in `_exit_tree()`)
- `bool is_steamvr_initialized()` - Check if SteamVR initialized successfully

**Left Hand Getters:**
- `float get_left_thumb_curl()` - Returns 0.0 to 1.0
- `float get_left_index_curl()`
- `float get_left_middle_curl()`
- `float get_left_ring_curl()`
- `float get_left_pinky_curl()`

**Right Hand Getters:**
- `float get_right_thumb_curl()`
- `float get_right_index_curl()`
- `float get_right_middle_curl()`
- `float get_right_ring_curl()`
- `float get_right_pinky_curl()`

#### Return Values

All getter methods return `float` values:
- `0.0` = Finger fully open
- `1.0` = Finger fully closed/curled
- Values are clamped to the 0.0 - 1.0 range

If SteamVR is not initialized or controllers are not detected, all methods return `0.0`.

## Troubleshooting

### Build Issues

**Error: "Cannot find openvr_api.lib"**
- Ensure the `openvr` submodule is properly initialized
- Check that `openvr/lib/win64/openvr_api.lib` exists

**Error: "Cannot find godot-cpp headers"**
- Run `git submodule update --init --recursive`
- Ensure `godot-cpp` is built before building the extension

**SCons errors:**
- Make sure you're using a Visual Studio Command Prompt (Windows)
- Verify Python and SCons are properly installed: `scons --version`

### Runtime Issues

**"SteamVR runtime not installed" error:**
- Install SteamVR from Steam
- Ensure SteamVR is running before starting the demo

**"No HMD detected" warning:**
- Connect your VR headset
- Launch SteamVR and ensure it shows the headset as connected
- The extension will still initialize but return 0.0 values

**"Failed to set action manifest" error:**
- Ensure `action_manifest.json` exists in `addons/steamvr_fingers/actions/`
- Check file paths in the error message
- Verify the demo is running from the correct directory

**All finger values are 0.0:**
- Ensure Valve Index controllers are paired and tracked in SteamVR
- Check SteamVR status window for controller connectivity
- Verify controllers are turned on and have battery

### OpenVR Action System

The extension uses OpenVR's action system with skeletal input. The action manifest (`action_manifest.json`) and controller bindings (`knuckles_bindings.json`) are configured for Valve Index controllers.

If you need to customize bindings:
1. Edit `demo/addons/steamvr_fingers/actions/knuckles_bindings.json`
2. Restart the demo to reload the action manifest

## Technical Details

### Architecture

- **OpenVR Integration**: Uses `IVRSystem` and `IVRInput` interfaces
- **Skeletal Actions**: Reads finger curl data via `GetSkeletalSummaryData()`
- **Action Manifest**: Defines skeletal actions for left/right hands
- **Update Frequency**: Finger values update every frame in `_process()`

### Performance

- **Minimal overhead**: Direct OpenVR API calls with no unnecessary processing
- **Frame rate**: Updates at engine frame rate (typically 60+ FPS)
- **Memory**: ~1 KB for finger curl storage

### Platform Support

Currently built for:
- ✅ **Windows x64** - Fully tested
- ⚠️ **Linux x64** - Build system ready, requires testing
- ❌ **macOS** - OpenVR support limited, not recommended

## License

This project uses:
- **godot-cpp**: MIT License
- **OpenVR**: BSD 3-Clause License

Please ensure compliance with both licenses when using this extension.

## Contributing

Contributions welcome! Areas for improvement:
- Linux testing and fixes
- Additional controller support (Vive, Oculus Touch with skeletal tracking)
- Performance optimizations
- Additional hand pose detection

## Credits

- **Godot Engine** - https://godotengine.org
- **Valve OpenVR SDK** - https://github.com/ValveSoftware/openvr
- **godot-cpp** - https://github.com/godotengine/godot-cpp

## Support

For issues, questions, or feature requests, please open an issue on GitHub.

---

**Happy VR Development!** 🎮✋
