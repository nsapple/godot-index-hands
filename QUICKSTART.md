# Quick Start Guide

Get up and running with SteamVR Fingers in 5 minutes!

## Prerequisites

- Windows 10/11 with Visual Studio 2019+
- Python 3.6+ and SCons (`pip install scons`)
- Godot 4.3
- SteamVR with Valve Index controllers

## Build Steps

### Windows

1. **Clone with submodules:**
   ```bash
   git clone --recursive https://github.com/yourusername/steamvr-fingers-extension.git
   cd steamvr-fingers-extension
   ```

2. **Run the build script:**
   ```bash
   build.bat
   ```

   This will build everything automatically. Wait for completion (~5-10 minutes first time).

### Linux

1. **Clone with submodules:**
   ```bash
   git clone --recursive https://github.com/yourusername/steamvr-fingers-extension.git
   cd steamvr-fingers-extension
   ```

2. **Run the build script:**
   ```bash
   chmod +x build.sh
   ./build.sh
   ```

## Run the Demo

1. Open Godot 4.3
2. Click **Import** and select `demo/project.godot`
3. Ensure SteamVR is running
4. Press **F5**

You should see real-time finger tracking data for both hands!

## Use in Your Project

Copy the `demo/addons/steamvr_fingers/` folder to your project and use:

```gdscript
extends Node

var finger_tracker: SteamVRFingers

func _ready():
    finger_tracker = SteamVRFingers.new()
    add_child(finger_tracker)

func _process(delta):
    var curl = finger_tracker.get_left_index_curl()
    print("Left index curl: ", curl)
```

## Troubleshooting

**Build fails?**
- Use Visual Studio Command Prompt on Windows
- Ensure submodules are initialized: `git submodule update --init --recursive`

**Demo shows 0.0 values?**
- Make sure SteamVR is running
- Check that Index controllers are connected and tracked
- Look for errors in Godot's output console

For detailed information, see [README.md](README.md).
