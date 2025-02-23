#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_GOOGLE_include_directive : enable

#include "raycommon.glsl"
#include "host_device.glsl"

hitAttributeEXT vec2 attribs;

layout(location = 0) rayPayloadInEXT hitPayload prd;

layout(set = 0, binding = 3) readonly buffer VertexBuffer { Vertex v[]; };
layout(set = 0, binding = 4) buffer IndexBuffer { uint i[]; };
layout(set = 0, binding = 5) readonly buffer ObjDescs { ObjDesc obj_desc[]; };

void main() {
  uint ind_offset = obj_desc[gl_InstanceID].index_offset;

  // Vertex of the triangle
  Vertex v0 = v[i[ind_offset + 3 * gl_PrimitiveID]];
  Vertex v1 = v[i[ind_offset + 3 * gl_PrimitiveID + 1]];
  Vertex v2 = v[i[ind_offset + 3 * gl_PrimitiveID + 2]];

  const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);

  // Computing the coordinates of the hit position
  const vec3 pos      = v0.pos * barycentrics.x + v1.pos * barycentrics.y + v2.pos * barycentrics.z;
  const vec3 worldPos = vec3(gl_ObjectToWorldEXT * vec4(pos, 1.0));  // Transforming the position to world space

  // Computing the normal at hit position
  const vec3 nrm      = v0.nrm * barycentrics.x + v1.nrm * barycentrics.y + v2.nrm * barycentrics.z;
  const vec3 worldNrm = normalize(vec3(nrm * gl_WorldToObjectEXT));  // Transforming the normal to world space

    // Vector toward the light
  vec3  L;
  //float lightIntensity = pcRay.lightIntensity;
  float lightDistance  = 100000.0;

  // Point light
  // if(pcRay.lightType == 0)
  // {
  //     vec3 lDir      = pcRay.lightPosition - worldPos;
  //     lightDistance  = length(lDir);
  //     lightIntensity = pcRay.lightIntensity / (lightDistance * lightDistance);
  //     L              = normalize(lDir);
  // }
  // else  // Directional light
  // {
  //     L = normalize(pcRay.lightPosition);
  // }

  vec3 lightPosition = vec3(1, 1, 1);

  L = normalize(lightPosition);

  // Lambertian
  float dotNL = max(dot(worldNrm, L), 0.0);
  vec3  c     = vec3(1, 1, 1) * dotNL;

  prd.hitValue = c;

  //prd.hitValue = vec3(0.0, 0.0, 1.0);
}