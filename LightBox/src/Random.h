#pragma once

#include <glm/glm.hpp>

#include <random>
#include <cstdlib>

#include "Vector3.h"

namespace LightBox {

	class Random
	{
	public:
		static void Init()
		{
			s_RandomEngine.seed(std::random_device()());
		}
		static uint32_t UInt()
		{
			return s_Distribution(s_RandomEngine);
		}
		static uint32_t UInt(uint32_t min, uint32_t max)
		{
			return min + (s_Distribution(s_RandomEngine) % (max - min + 1));
		}
		static float Float()
		{
			return (float)s_Distribution(s_RandomEngine) / (float)std::numeric_limits<uint32_t>::max();
		}
		static float Float(float min, float max)
		{
			return Float() * (max - min) + min;
		}
		static glm::vec3 Vec3()
		{
			return glm::vec3(Float(), Float(), Float());
		}
		static glm::vec3 Vec3(float min, float max)
		{
			return glm::vec3(Float() * (max - min) + min, Float() * (max - min) + min, Float() * (max - min) + min);
		}
		static glm::vec3 InUnitSphere()
		{
			return glm::normalize(Vec3(-1.0f, 1.0f));
		}
		static Vector3 UnitVector()
		{
			return RandomVector3(-1.0f, 1.0f).Normalize();
		}
		static Vector3 RandomVector3(float min, float max)
		{
			return Vector3(Float() * (max - min) + min, Float() * (max - min) + min, Float() * (max - min) + min);
		}
		static Vector3 GetRandomUnitVector3() {
			// The Cherno
			return RandomVector3(-1.0f, 1.0f).Normalize();
		}
		// RAY TRACING IN ONE WEEKEND
		/////////////////////////////
		static inline double random_double() {
			// Returns a random real in [0,1).
			return rand() / (RAND_MAX + 1.0);
		}
		static inline double random_double(double min, double max) {
			// Returns a random real in [min,max).
			return min + (max - min) * random_double();
		}
		inline static int random_int(int min, int max) {
			// Returns a random integer in [min,max].
			return static_cast<int>(random_double(min, max + 1));
		}
		static float GetRandomFloat() {
			static std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
			static std::mt19937 generator;
			
			return distribution(generator);
		}
		static float GetRandomFloat(double min, double max) {
			return (float)(min + (max - min) * GetRandomFloat());
		}
	private:
		static thread_local std::mt19937 s_RandomEngine;
		static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;
	};

}


