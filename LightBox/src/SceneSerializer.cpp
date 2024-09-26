#include "SceneSerializer.h"

#include <yaml-cpp/yaml.h>

namespace YAML
{
	template<>
	struct convert<LightBox::Vector3>
	{
		static Node encode(const LightBox::Vector3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, LightBox::Vector3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};
}

namespace LightBox
{
	YAML::Emitter& operator<<(YAML::Emitter& out, const Vector3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	void SceneSerializer::Serialize(const std::string& filepath) const
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "DefaultScene";

		out << YAML::Key << "Camera";
		out << YAML::BeginMap; // Camera
		out << YAML::Key << "position" << m_Camera.m_Position;
		out << YAML::Key << "forward_direction" << m_Camera.m_ForwardDirection;
		out << YAML::Key << "fov" << YAML::Value << m_Camera.m_Fov;
		out << YAML::Key << "near_plane" << YAML::Value << m_Camera.m_NearPlane;
		out << YAML::Key << "far_plane" << YAML::Value << m_Camera.m_FarPlane;
		out << YAML::EndMap; // Camera

		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	bool SceneSerializer::Deserialize(const std::string& filepath) const
	{
		std::ifstream stream(filepath);
		std::stringstream strstream;
		strstream << stream.rdbuf();

		YAML::Node data = YAML::Load(strstream.str());
		if (!data["Scene"])
			return false;

		std::string scene_name = data["Scene"].as<std::string>();

		auto camera = data["Camera"];
		if (!camera)
			return false;

		m_Camera.m_Fov = camera["fov"].as<float>();
		m_Camera.m_NearPlane = camera["near_plane"].as<float>();
		m_Camera.m_FarPlane = camera["far_plane"].as<float>();
		m_Camera.m_Position = camera["position"].as<Vector3>();
		m_Camera.m_ForwardDirection = camera["forward_direction"].as<Vector3>();

		m_Camera.Refresh();

		return true;
	}
}
