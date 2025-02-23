#include "Node.h"

namespace LightBox {
	Node::Node(std::vector<Triangle>& tris, int32_t triangle_index, int32_t triangle_count)
		: m_Tris(tris), m_TriangleIndex(triangle_index), m_TriangleCount(triangle_count)
	{
		for (int32_t i = triangle_index; i < triangle_index + triangle_count; i++)
			GrowToInclude(tris[i]);

		m_BoundingBox = AABB(m_Min, m_Max);
	}

	void Node::Split(int32_t depth)
	{
		/*if (depth == 0)
			return;*/
		if (m_TriangleCount < 15)
			return;

		uint32_t longest_edge_id = GetLongestEdgeId();
		float split_coord = m_Min[longest_edge_id] + (m_Max[longest_edge_id] - m_Min[longest_edge_id]) / 2;

		float object_median = 0;
		for (int32_t i = m_TriangleIndex; i < m_TriangleIndex + m_TriangleCount; i++) {
			Vector3* verts = m_Tris[i].vertices;
			float tri_center = verts[0][longest_edge_id] + verts[1][longest_edge_id]
				+ verts[2][longest_edge_id];
			tri_center = tri_center / 3;

			object_median += tri_center;
		}
		object_median = object_median / m_TriangleCount;
		split_coord = object_median;

		int32_t sideA_tri_count = 0;

		for (int32_t i = m_TriangleIndex; i < m_TriangleIndex + m_TriangleCount; i++)
		{
			float tri_center = m_Tris[i].vertices[0][longest_edge_id] + m_Tris[i].vertices[1][longest_edge_id]
				+ m_Tris[i].vertices[2][longest_edge_id];
			tri_center = tri_center / 3;

			bool isSideA = tri_center < split_coord;
			if (isSideA)
			{
				int32_t sideA_array_index = m_TriangleIndex + sideA_tri_count;
				Triangle temp = m_Tris[sideA_array_index];

				m_Tris[sideA_array_index] = m_Tris[i];
				m_Tris[i] = temp;

				sideA_tri_count++;
			}
		}

		m_ChildA = new Node(m_Tris, m_TriangleIndex, sideA_tri_count);
		m_ChildB = new Node(m_Tris, m_TriangleIndex + sideA_tri_count, m_TriangleCount - sideA_tri_count);

		if (sideA_tri_count == 0 || m_TriangleCount - sideA_tri_count == 0)
		{
			std::cout << "catch!\n";
			return;
		}

		m_ChildA->Split(depth - 1);
		m_ChildB->Split(depth - 1);
	}

	bool Node::Hit(const Ray& ray, Interval ray_t, HitRecord& rec) const
	{
		if (!m_BoundingBox.Hit(ray, ray_t))
			return false;

		bool hit_anything = false;

		if (m_ChildA == nullptr && m_ChildB == nullptr)
		{
			float closest_so_far = ray_t.max;
			for (int32_t i = m_TriangleIndex; i < m_TriangleIndex + m_TriangleCount; i++)
			{
				if (m_Tris[i].Hit(ray, Interval(ray_t.min, closest_so_far), rec))
				{
					closest_so_far = rec.t;
					hit_anything = true;
				}
			}
		}
		else
		{
			bool hit_A = m_ChildA->Hit(ray, ray_t, rec);
			bool hit_B = m_ChildB->Hit(ray, Interval(ray_t.min, hit_A ? rec.t : ray_t.max), rec);

			hit_anything = hit_A || hit_B;
		}

		return hit_anything;
	}

	void Node::GetBVHGeometryData(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, int32_t& box_count)
	{
		// TODO can be optimized (a node always has two children, so it does not make sense to check if both are not null)
		if (m_ChildA != nullptr && m_ChildB != nullptr)
		{
			m_ChildA->GetBVHGeometryData(vertices, indices, box_count);
			m_ChildB->GetBVHGeometryData(vertices, indices, box_count);

			Vector3 color_01(Random::Float(0, 1.f),
				Random::Float(0, 1.f),
				Random::Float(0, 1.f));

			int32_t start_index = vertices.size() - 16;
			for (int32_t i = start_index; i < vertices.size(); i++)
				vertices[i].col = {color_01.x, color_01.y, color_01.z};
		}

		Vector3 color(1.f);

		std::vector<Vertex> local_vertices = { {{m_Min.x, m_Min.y, m_Max.z}, {color.x, color.y, color.z} },
		{{m_Min.x, m_Max.y, m_Max.z}, {color.x, color.y, color.z} },
		{{m_Max.x, m_Max.y, m_Max.z}, {color.x, color.y, color.z} },
		{{m_Max.x, m_Min.y, m_Max.z}, {color.x, color.y, color.z} },
		{{m_Min.x, m_Min.y, m_Min.z}, {color.x, color.y, color.z} },
		{{m_Min.x, m_Max.y, m_Min.z}, {color.x, color.y, color.z} },
		{{m_Max.x, m_Max.y, m_Min.z}, {color.x, color.y, color.z} },
		{{m_Max.x, m_Min.y, m_Min.z}, {color.x, color.y, color.z} }
		 };
		for (int32_t i = 0; i < 8; i++)
		{
			vertices.push_back(local_vertices[i]);
		}
		std::vector<uint32_t> local_indices = { 0, 2, 1, 0, 3, 2,
		3, 2, 6, 3, 6, 7,
		7, 6, 5, 7, 5, 4,
		4, 5, 1, 4, 1, 0,
		1, 5, 6, 1, 6, 2,
		4, 0, 3, 4, 3, 7 };
		for (int32_t i = 0; i < 36; i++)
		{
			indices.push_back(local_indices[i] + box_count * 8);
		}

		box_count++;
	}

	void Node::GrowToInclude(const Triangle& tri)
	{
		const Vector3* verts = tri.vertices;

		for (int32_t i = 0; i < 3; i++) {
			m_Min = Vector3::Min(m_Min, verts[i]);
			m_Max = Vector3::Max(m_Max, verts[i]);
		}
		Vector3 delta = m_Max - m_Min;
		for (int i = 0; i < 3; i++)
		{
			if (delta[i] == 0.f)
			{
				m_Min[i] -= 0.001f;
				m_Max[i] += 0.001f;
			}
		}
	}

	uint32_t Node::GetLongestEdgeId() const
	{
		Vector3 delta = m_Max - m_Min;
		uint32_t id = delta.y >= delta.x ? 1 : 0;
		if (delta.z >= delta[id])
			id = 2;

		return id;
	}
}
