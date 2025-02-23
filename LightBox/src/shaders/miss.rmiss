#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_GOOGLE_include_directive : enable

#include "raycommon.glsl"

layout(location = 0) rayPayloadInEXT hitPayload prd;

void main() {
    // prd.hitValue = vec3(1.0, 1.0, 0.0);

    // prd.hitValue = vec3(0.0, 0.0, 0.0);
    // shadow ray has not hit an object
    prd.shadowRayMiss = true; 

    vec3 rayDirection = gl_WorldRayDirectionEXT;

    vec3 colorA = vec3(0.5f, 0.7f, 1.0f);
    vec3 colorB = vec3(1, 1, 1);

    float t = 0.5 * (rayDirection.y + 1);

    if (rayDirection.y > 0.0)
        prd.hitValue = mix(colorA, colorB, t);
    else
        prd.hitValue = vec3(0.23, 0.2, 0.2);
}