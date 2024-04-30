@echo off

pushd ..
pushd ..
pushd ..
@echo on
vendor\glslc.exe LightBox\src\shaders\simple_shader.vert -o LightBox\src\shaders\simple_shader.vert.spv
vendor\glslc.exe LightBox\src\shaders\simple_shader.frag -o LightBox\src\shaders\simple_shader.frag.spv

vendor\glslc.exe LightBox\src\shaders\raster.vert -o LightBox\src\shaders\raster.vert.spv
vendor\glslc.exe LightBox\src\shaders\raster.frag -o LightBox\src\shaders\raster.frag.spv

vendor\glslc.exe LightBox\src\shaders\triangle.vert -o LightBox\src\shaders\triangle.vert.spv
vendor\glslc.exe LightBox\src\shaders\triangle.frag -o LightBox\src\shaders\triangle.frag.spv

vendor\glslc.exe LightBox\src\shaders\main.vert -o LightBox\src\shaders\main.vert.spv
vendor\glslc.exe LightBox\src\shaders\main.frag -o LightBox\src\shaders\main.frag.spv

vendor\glslc.exe LightBox\src\shaders\raygen.rgen -o LightBox\src\shaders\raygen.rgen.spv
vendor\glslc.exe LightBox\src\shaders\miss.rmiss -o LightBox\src\shaders\miss.rmiss.spv
vendor\glslc.exe LightBox\src\shaders\closesthit.rchit -o LightBox\src\shaders\closesthit.rchit.spv

@echo off
popd
popd
popd
pause