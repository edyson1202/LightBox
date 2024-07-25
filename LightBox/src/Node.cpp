#include "Node.h"

namespace LightBox {
	Node::Node(std::vector<Triangle>& tris, int32_t triangle_index, int32_t triangle_count)
		: m_TriangleIndex(triangle_index),
		m_TriangleCount(triangle_count)
	{
		m_Tris = tris;

		for (int32_t i = triangle_index; i < triangle_index + triangle_count; i++)
		{
			GrowToInclude(tris[i]);
		}

		m_BoundingBox = AABB(m_Min, m_Max);
	}

	void Node::Split(int32_t depth)
	{
		if (m_TriangleCount < 100)
			return;

		uint32_t longest_edge_id = GetLongestEdgeId();
		float split_coord = m_Min[longest_edge_id] + (m_Max[longest_edge_id] - m_Min[longest_edge_id]) / 2;
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

		std::cout << "Side A index: " << m_TriangleIndex << std::endl;
		std::cout << "Side B index: " << m_TriangleIndex + sideA_tri_count << std::endl;
		
		std::cout << "Side A triangle count: " << sideA_tri_count << std::endl;
		std::cout << "Side B triangle count: " << m_TriangleCount - sideA_tri_count << std::endl;

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

	void Node::GrowToInclude(const Triangle& tri)
	{
		Vector3* verts = (Vector3*)&tri.vertices[0];

		for (int32_t i = 0; i < 3; i++) {
			m_Min = Vector3::Min(m_Min, verts[i]);
			m_Max = Vector3::Max(m_Max, verts[i]);
		}
	}

	uint32_t Node::GetLongestEdgeId() const
	{
		// TODO can be optimized by subtracting m_Min from m_Max and use the components of the resulting vector
		uint32_t id = 0;
		if (std::abs(m_Max.y - m_Min.y) >= std::abs(m_Max.x - m_Min.x))
			id = 1;

		if (std::abs(m_Max.z - m_Min.z) >= std::abs(m_Max[id] - m_Min[id]))
			id = 2;

		return id;
	}
}
