@echo off
echo Compiling shaders...
del *.spv
glslangValidator.exe -V pbr_shader.vert -o pbr_vert.spv
glslangValidator.exe -V pbr_shader.frag -o pbr_frag.spv

glslangValidator.exe -V sky_shader.vert -o sky_vert.spv
glslangValidator.exe -V sky_shader.frag -o sky_frag.spv

glslangValidator.exe -V tex_shader.vert -o tex_vert.spv
glslangValidator.exe -V tex_shader.frag -o tex_frag.spv

REM glslangValidator.exe -V red_shader.vert -o red_vert.spv
REM glslangValidator.exe -V red_shader.frag -o red_frag.spv

REM timeout 4
REM pause

IF %0 == "%~0" timeout 4
