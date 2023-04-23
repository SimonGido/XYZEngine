#pragma once
#include "XYZ/Core/Core.h"
#include "XYZ/Reflection/ReflectionUtils.h"

#include <glm/glm.hpp>


namespace XYZ {

	namespace Math {

		struct Plane
		{
			glm::vec3 Normal = { 0.f, 1.f, 0.f }; // unit vector
			float     Distance = 0.f;        // Distance with origin

			Plane() = default;

			Plane(const glm::vec3& p1, const glm::vec3& norm)
				:
				Normal(glm::normalize(norm)),
				Distance(glm::dot(Normal, p1))
			{}

			float GetSignedDistanceToPlane(const glm::vec3& point) const
			{
				return glm::dot(Normal, point) - Distance;
			}
		};

		struct Frustum
		{
			Plane TopFace;
			Plane BottomFace;

			Plane RightFace;
			Plane LeftFace;

			Plane FarFace;
			Plane NearFace;
		};

		XYZ_API std::pair<glm::vec2, glm::vec2> CalculateVelocitiesAfterContact(
			const glm::vec2& firstVelocity, const glm::vec2& secondVelocity, float firstMass, float secondMass
		);

		XYZ_API glm::vec2 CalculateMirrorVector(const glm::vec2& dir, const glm::vec2& hitNormal);

		XYZ_API float PythagoreanSolve(float a, float b);

		XYZ_API void DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
	
		XYZ_API glm::vec3 TransformToTranslation(const glm::mat4& transform);

		inline float Sign(const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3)
		{
			return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
		}

		inline bool PointInTriangle(const glm::vec2& pt, const glm::vec2& v1, const glm::vec2& v2, const glm::vec2& v3)
		{
			float d1, d2, d3;
			bool has_neg, has_pos;

			d1 = Sign(pt, v1, v2);
			d2 = Sign(pt, v2, v3);
			d3 = Sign(pt, v3, v1);

			has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
			has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

			return !(has_neg && has_pos);
		}

		XYZ_API std::tuple<glm::vec3, glm::vec3, glm::vec3> DecomposeTransform(const glm::mat4& transform);

		XYZ_API std::tuple<glm::vec3, glm::quat, glm::vec3> DecomposeTransformQuat(const glm::mat4& transform);
	
	
		XYZ_API int32_t RoundUp(int32_t numToRound, int32_t multiple);
	

		template <size_t N, size_t K>
		inline constexpr float ComputeBinominal()
		{
			if constexpr (N == K)
				return 1.0f;

			float value = 1.0f;

			For<K>([&value](auto j) {

				auto i = j.value + 1;
				value = value * ((N + 1 - i) / i);
			});

			return value;
		}

		template <size_t N>
		inline std::array<float, N> ComputeBinominals()
		{
			std::array<float, N> binomials;

			For<N>([&](auto i) {

				binomials[i.value] = ComputeBinominal<N - 1, i.value>();
			});
			return binomials;
		}


		template <size_t NumPoints>
		inline glm::vec2 ComputeBezierCurve2D(const std::array<glm::vec2, NumPoints>& points, float t)
		{
			std::array<float, NumPoints> binomials = ComputeBinominals<NumPoints>();
			const size_t n = points.size() - 1;

			float bCurveXt{ 0.0f };
			float bCurveYt{ 0.0f };

			for (size_t i = 0; i <= n; ++i)
			{
				bCurveXt += binomials[i] * std::pow((1 - t), (n - i)) * std::pow(t, i) * points.x[i];
				bCurveYt += binomials[i] * std::pow((1 - t), (n - i)) * std::pow(t, i) * points.y[i];

			}
			return glm::vec2{ bCurveXt, bCurveYy };
		}
	}
}