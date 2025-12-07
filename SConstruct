#!/usr/bin/env python
import os
import sys

env = SConscript("godot-cpp/SConstruct")

# For reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=["src/", "openvr/headers/"])
sources = Glob("src/*.cpp")

# OpenVR library linking
if env["platform"] == "windows":
    # Windows x64
    openvr_lib_path = "openvr/lib/win64/"
    env.Append(LIBPATH=[openvr_lib_path])
    env.Append(LIBS=["openvr_api"])

    # Copy openvr_api.dll to demo bin directory after build
    openvr_dll_source = openvr_lib_path + "openvr_api.dll"
    openvr_dll_dest = "demo/addons/steamvr_fingers/bin/openvr_api.dll"

elif env["platform"] == "linux":
    # Linux x64
    openvr_lib_path = "openvr/lib/linux64/"
    env.Append(LIBPATH=[openvr_lib_path])
    env.Append(LIBS=["openvr_api"])

    # Copy openvr_api.so to demo bin directory after build
    openvr_so_source = openvr_lib_path + "libopenvr_api.so"
    openvr_so_dest = "demo/addons/steamvr_fingers/bin/libopenvr_api.so"

elif env["platform"] == "macos":
    # macOS
    openvr_lib_path = "openvr/lib/osx32/"
    env.Append(LIBPATH=[openvr_lib_path])
    env.Append(LIBS=["openvr_api"])

# Determine output path
if env["platform"] == "macos":
    library_name = "libsteamvr_fingers.{}.{}.framework/libsteamvr_fingers.{}.{}".format(
        env["platform"], env["target"], env["platform"], env["target"]
    )
else:
    library_name = "libsteamvr_fingers{}{}".format(env["suffix"], env["SHLIBSUFFIX"])

library_path = "demo/addons/steamvr_fingers/bin/{}".format(library_name)

# Build the library
library = env.SharedLibrary(
    library_path,
    source=sources,
)

# Copy OpenVR DLL/SO to demo bin after building
if env["platform"] == "windows":
    dll_copy = env.Command(
        "demo/addons/steamvr_fingers/bin/openvr_api.dll",
        "openvr/bin/win64/openvr_api.dll",
        Copy("$TARGET", "$SOURCE")
    )
    env.Depends(library, dll_copy)
    env.AlwaysBuild(dll_copy)

elif env["platform"] == "linux":
    so_copy = env.Command(
        "demo/addons/steamvr_fingers/bin/libopenvr_api.so",
        "openvr/bin/linux64/libopenvr_api.so",
        Copy("$TARGET", "$SOURCE")
    )
    env.Depends(library, so_copy)
    env.AlwaysBuild(so_copy)

Default(library)
