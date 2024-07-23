#pragma once
#include "AABB.h"
#include "Triangle.h"

#include <vector>


namespace LightBox
{
	class Node
	{
	public:
		Node() = default;
		Node(std::vector<Triangle>& tris, int32_t triangle_index, int32_t triangle_count);

		void Split(int32_t depth);

		bool Hit(const Ray& ray, Interval ray_t, HitRecord& rec) const;

	private:
		void GrowToInclude(const Triangle& tri);

		uint32_t GetLongestEdgeId() const;

	public:
		Node* m_ChildA = nullptr;
		Node* m_ChildB = nullptr;

		std::vector<Triangle> m_Tris;
		int32_t m_TriangleIndex;
		int32_t m_TriangleCount;

		AABB m_BoundingBox;


		Vector3 m_Min = Vector3(std::numeric_limits<int32_t>::max() - 1000);
		Vector3 m_Max = Vector3(std::numeric_limits<int32_t>::min() + 1000);
	private:


	};
}


