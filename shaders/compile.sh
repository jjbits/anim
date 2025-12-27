#!/bin/bash

# Compile GLSL shaders to SPIR-V
# Requires glslc (from Vulkan SDK) or glslangValidator

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Use glslc if available, otherwise glslangValidator
if command -v glslc &> /dev/null; then
    COMPILER="glslc"
    compile() {
        glslc "$1" -o "$2"
    }
elif command -v glslangValidator &> /dev/null; then
    COMPILER="glslangValidator"
    compile() {
        glslangValidator -V "$1" -o "$2"
    }
else
    echo "Error: No GLSL compiler found. Install Vulkan SDK."
    exit 1
fi

echo "Using $COMPILER"

compile triangle.vert triangle.vert.spv
compile triangle.frag triangle.frag.spv

echo "Shader compilation complete"
