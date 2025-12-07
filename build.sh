#!/bin/bash

echo "===================================="
echo "Building SteamVR Fingers Extension"
echo "===================================="
echo ""

echo "Step 1: Building godot-cpp (Debug)..."
cd godot-cpp
scons platform=linux target=template_debug -j$(nproc)
if [ $? -ne 0 ]; then
    echo "ERROR: Failed to build godot-cpp debug"
    cd ..
    exit 1
fi

echo ""
echo "Step 2: Building godot-cpp (Release)..."
scons platform=linux target=template_release -j$(nproc)
if [ $? -ne 0 ]; then
    echo "ERROR: Failed to build godot-cpp release"
    cd ..
    exit 1
fi
cd ..

echo ""
echo "Step 3: Building extension (Debug)..."
scons platform=linux target=template_debug -j$(nproc)
if [ $? -ne 0 ]; then
    echo "ERROR: Failed to build extension debug"
    exit 1
fi

echo ""
echo "Step 4: Building extension (Release)..."
scons platform=linux target=template_release -j$(nproc)
if [ $? -ne 0 ]; then
    echo "ERROR: Failed to build extension release"
    exit 1
fi

echo ""
echo "===================================="
echo "Build Complete!"
echo "===================================="
echo ""
echo "Built files are in: demo/addons/steamvr_fingers/bin/"
echo ""
echo "You can now open the demo project in Godot 4.3"
echo ""
