#!/bin/bash
echo "Cleanup..."
rm spirv/*

echo "Compiling shaders..."

glslangValidator -V shader.vert -o spirv/shader_vert.spv
glslangValidator -V shader.frag -o spirv/shader_frag.spv

echo "Done"
sleep 2
