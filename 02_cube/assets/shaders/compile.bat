REM Compile shaders on Windows

echo "Cleanup.."
del spirv/*

echo "Compiling shaders..."
glslangValidator.exe -V shader.vert -o spirv/shader_vert.spv
glslangValidator.exe -V shader.frag -o spirv/shader_frag.spv
pause
