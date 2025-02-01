#pragma once
#include <cassert>

#include "Math.h"
#include "vector"

namespace dae
{
#pragma region GEOMETRY
	struct Sphere
	{
		Vector3 origin{};
		float radius{};
		float radiusSqrt{};
		unsigned char materialIndex{ 0 };
	};

	struct Plane
	{
		Vector3 origin{};
		Vector3 normal{};

		unsigned char materialIndex{ 0 };
	};

	enum class TriangleCullMode
	{
		FrontFaceCulling,
		BackFaceCulling,
		NoCulling
	};

	struct Triangle
	{
		Triangle() = default;
		Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2, const Vector3& _normal):
			v0{_v0}, v1{_v1}, v2{_v2}, normal{_normal.Normalized()}{}

		Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2) :
			v0{ _v0 }, v1{ _v1 }, v2{ _v2 }
		{
			const Vector3 edgeV0V1 = v1 - v0;
			const Vector3 edgeV0V2 = v2 - v0;
			//normal = Vector3::Cross(edgeV0V1, edgeV0V2).Normalized();
		}

		Vector3 v0{};
		Vector3 v1{};
		Vector3 v2{};

		Vector3 normal{};

		TriangleCullMode cullMode{};
		unsigned char materialIndex{};
	};

	struct TriangleMesh
	{
		TriangleMesh() = default;
		TriangleMesh(const std::vector<Vector3>& _positions, const std::vector<int>& _indices, TriangleCullMode _cullMode):
		positions(_positions), indices(_indices), cullMode(_cullMode)
		{
			//Calculate Normals
			//CalculateNormals();

			//Update Transforms
			UpdateTransforms();
		}

		/*TriangleMesh(const std::vector<Vector3>& _positions, const std::vector<int>& _indices, const std::vector<Vector3>& _normals, TriangleCullMode _cullMode) :
			positions(_positions), indices(_indices), normals(_normals), cullMode(_cullMode)
		{
			UpdateTransforms();
		}*/

		std::vector<Vector3> positions{};
		//std::vector<Vector3> normals{};
		std::vector<int> indices{};
		unsigned char materialIndex{};

		TriangleCullMode cullMode{TriangleCullMode::BackFaceCulling};

		Matrix rotationTransform{};
		Matrix translationTransform{};
		Matrix scaleTransform{};

		Vector3 minAABB;
		Vector3 maxAABB;

		Vector3 transformedMinAABB;
		Vector3 transformedMaxAABB;

		float m_Pitch{};
		float m_Yaw{};
		float m_Roll{};

		std::vector<Vector3> transformedPositions{};
		//std::vector<Vector3> transformedNormals{};

		void UpdateAABB()
		{
			if(positions.size() > 0)
			{
				minAABB = positions[0];
				maxAABB = positions[0];
				for (size_t i = 0; i < positions.size(); i++)
				{
					minAABB = Vector3::Min(minAABB, positions[i]);
					maxAABB = Vector3::Max(maxAABB, positions[i]);
				}
			}
			
		}

		void UpdateTransformedAABB(const Matrix& finalTransform)
		{
			Vector3 tMinAABB = finalTransform.TransformPoint(minAABB);
			Vector3 tMaxAABB = tMinAABB;

			Vector3 tAABB = finalTransform.TransformPoint(maxAABB.x, minAABB.y, minAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			tAABB = finalTransform.TransformPoint(maxAABB.x, minAABB.y, maxAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			tAABB = finalTransform.TransformPoint(minAABB.x, minAABB.y, maxAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			tAABB = finalTransform.TransformPoint(minAABB.x, maxAABB.y, minAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			tAABB = finalTransform.TransformPoint(maxAABB.x, maxAABB.y, minAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			tAABB = finalTransform.TransformPoint(maxAABB);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			tAABB = finalTransform.TransformPoint(minAABB.x, maxAABB.y, minAABB.z);
			tMinAABB = Vector3::Min(tAABB, tMinAABB);
			tMaxAABB = Vector3::Max(tAABB, tMaxAABB);

			transformedMinAABB = tMinAABB;
			transformedMaxAABB = tMaxAABB;
		}

		void Translate(const Vector3& translation)
		{
			translationTransform = Matrix::CreateTranslation(translation);
		}

		void RotateY(float yaw)
		{
			m_Yaw = yaw;
			UpdateRotate();
		}

		void RotateX(float pitch)
		{
			m_Pitch = pitch;
			UpdateRotate();
		}

		void RotateZ(float roll)
		{
			m_Roll = roll;
			UpdateRotate();
		}

		void UpdateRotate()
		{
			rotationTransform = Matrix::CreateRotation(m_Pitch, m_Yaw, m_Roll);;
		}

		void Scale(const Vector3& scale)
		{
			scaleTransform = Matrix::CreateScale(scale);
		}

		void AppendTriangle(const Triangle& triangle, bool ignoreTransformUpdate = false)
		{
			int startIndex = static_cast<int>(positions.size());

			positions.emplace_back(triangle.v0);
			positions.emplace_back(triangle.v1);
			positions.emplace_back(triangle.v2);

			indices.push_back(startIndex);
			indices.push_back(++startIndex);
			indices.push_back(++startIndex);

			//normals.push_back(triangle.normal);

			//Not ideal, but making sure all vertices are updated
			if(!ignoreTransformUpdate)
				UpdateTransforms();
		}

		//void CalculateNormals()
		//{
		//	//assert(false && "No Implemented Yet!");
		//	const int trianglesAmount{ static_cast<int>(indices.size() / 3) };
		//	Vector3 edgeA{};
		//	Vector3 edgeB{};
		//	Vector3 vertA, vertB, vertC;
		//	normals.reserve(trianglesAmount);
		//	for (int i = 0; i < trianglesAmount; ++i)
		//	{
		//		vertA = positions[indices[i * 3]];
		//		vertB = positions[indices[i * 3 + 1]];
		//		vertC = positions[indices[i * 3 + 2]];
		//		edgeA = vertB - vertA;
		//		edgeB = vertC - vertA;
		//		normals.emplace_back(Vector3::Cross(edgeA, edgeB).Normalized());
		//	}
		//}

		void UpdateTransforms()
		{
			//assert(false && "No Implemented Yet!");
			//Calculate Final Transform 
			const auto finalTransform = scaleTransform * rotationTransform * translationTransform;

			//Transform Positions (positions > transformedPositions)
			transformedPositions.clear();
			transformedPositions.reserve(positions.size());
			for (const Vector3& pos : positions)
			{
				transformedPositions.emplace_back(finalTransform.TransformPoint(pos));
			}
			UpdateTransformedAABB(finalTransform);
			//Transform Normals (normals > transformedNormals)
			/*transformedNormals.clear();
			transformedNormals.reserve(normals.size());
			for (const Vector3& norm : normals)
			{
				transformedNormals.emplace_back(finalTransform.TransformVector(norm));
			}*/
		}
	};
#pragma endregion
#pragma region LIGHT
	enum class LightType
	{
		Point,
		Directional
	};

	struct Light
	{
		Vector3 origin{};
		Vector3 direction{};
		ColorRGB color{};
		float intensity{};

		LightType type{};
	};
#pragma endregion
#pragma region MISC
	struct Ray
	{
		Vector3 origin{};
		Vector3 direction{};

		float min{ 0.0001f };
		float max{ FLT_MAX };
	};

	struct HitRecord
	{
		Vector3 origin{};
		Vector3 normal{};
		float t = FLT_MAX;

		bool didHit{ false };
		unsigned char materialIndex{ 0 };

	};
#pragma endregion
}