

// using vec2 = LightBox::Vector2;
// using vec3 = LightBox::Vector3;
// using vec4 = LightBox::Vector4;
// using mat4 = LightBox::Mat4;
// using uint = unsigned int;

struct Vertex
{
  vec3 pos;
  vec3 nrm;
  vec2 texCoord;
};

struct Vertex2
{
  vec3 nrm;
};

struct ObjDesc
{
	uint index_offset;
	uint primitive_count;
};