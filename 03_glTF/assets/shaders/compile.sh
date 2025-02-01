#!/bin/bash
echo "Compiling PBR shaders..."
glslangValidator -V pbr_shader.vert -o spirv/pbr_vert.spv
glslangValidator -V pbr_shader.frag -o spirv/pbr_frag.spv
echo

echo "Compiling Sky shaders..."
glslangValidator -V sky_shader.vert -o spirv/sky_vert.spv
glslangValidator -V sky_shader.frag -o spirv/sky_frag.spv
echo

echo "Compiling Tex shaders..."
glslangValidator -V tex_shader.vert -o spirv/tex_vert.spv
glslangValidator -V tex_shader.frag -o spirv/tex_frag.spv
echo

#echo "Compiling Red shaders..."
#glslangValidator -V red_shader.vert -o spirv/red_vert.spv
#glslangValidator -V red_shader.frag -o spirv/red_frag.spv
#echo

echo "Compiling Subpass 1 shaders..."
glslangValidator -V sub1.vert -o spirv/sub1_vert.spv
glslangValidator -V sub1.frag -o spirv/sub1_frag.spv
echo

echo "Done"
sleep 2

