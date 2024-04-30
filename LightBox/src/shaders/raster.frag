#version 450

// layout modifier specifies the index of the framebuffer
// the outColor variable is linked to the framebuffer at index 0
layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(fragColor, 1.0);
}