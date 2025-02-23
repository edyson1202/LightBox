glslc --target-spv=spv1.4 -o raygen.rgen.spv raygen.rgen
glslc --target-spv=spv1.4 -o closesthit.rchit.spv closesthit.rchit
glslc --target-spv=spv1.4 -o miss.rmiss.spv miss.rmiss

glslc --target-spv=spv1.4 -o shadows.rchit.spv shadows.rchit