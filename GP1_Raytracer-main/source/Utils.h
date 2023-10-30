#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

#define MOLLER_TRUMBORE

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W1
			const Vector3 rayToSphere{ sphere.origin - ray.origin };

			const float tCa{ Vector3::Dot(rayToSphere, ray.direction) };

			const float squaredRadius{ sphere.radius * sphere.radius };

			const float od{ (rayToSphere.SqrMagnitude() - tCa * tCa)};
			if (od >= squaredRadius) return false;
			//TODO: calculte sphere squared in sphere class
			const float tHc{ sqrt(squaredRadius - od) };

			const float tZero{ (tCa - tHc > ray.min) ? tCa - tHc : tCa + tHc };

			if (tZero < ray.min || tZero > ray.max) return false;

			if (!ignoreHitRecord)
			{
				const Vector3 intersectPoint{ray.origin + tZero * ray.direction };
				hitRecord.didHit = true;
				hitRecord.materialIndex = sphere.materialIndex;
				hitRecord.origin = intersectPoint;
				hitRecord.normal = (intersectPoint - sphere.origin).Normalized();
				hitRecord.t = tZero;
			}

			return true;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W1
			//assert(false && "No Implemented Yet!")
			const float denominator{ Vector3::Dot(ray.direction,plane.normal) };
			const float t{ Vector3::Dot(plane.origin - ray.origin,plane.normal) / denominator };
			if (t< ray.min || t> ray.max) return false;
			
			if (!ignoreHitRecord)
			{
				const Vector3 intersectPoint{ ray.origin + t * ray.direction };
				hitRecord.didHit = true;
				hitRecord.materialIndex = plane.materialIndex;
				hitRecord.t = t;
				hitRecord.origin = intersectPoint;
				hitRecord.normal = plane.normal;
			}
			return true;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
#ifdef MOLLER_TRUMBORE 
			const Vector3 edge1{ triangle.v1 - triangle.v0 };
			const Vector3 edge2{ triangle.v2 - triangle.v0 };
			const Vector3 h{ Vector3::Cross(ray.direction,edge2) };
			const float a{ Vector3::Dot(edge1,h) };

			switch (triangle.cullMode)
			{
			case TriangleCullMode::BackFaceCulling:
				if (a < FLT_EPSILON && !ignoreHitRecord) return false;
				
				break;
			case TriangleCullMode::FrontFaceCulling:
				if (a > FLT_EPSILON && !ignoreHitRecord) return false;
				if (a < FLT_EPSILON && ignoreHitRecord) return false;
				break;
			case TriangleCullMode::NoCulling:
				if (a > -FLT_EPSILON && a < FLT_EPSILON && !ignoreHitRecord) return false;
				if (a > FLT_EPSILON && ignoreHitRecord) return false;
				break;
			default:
				break;
			}
			
			const float f{ 1.f / a };
			const Vector3 s{ ray.origin - triangle.v0 };
			const float u{ f * Vector3::Dot(s,h) };

			if (u < 0.f || u >1.f) return false;

			const Vector3 q = { Vector3::Cross(s,edge1) };
			const float v{ f * Vector3::Dot(ray.direction,q) };

			if (v < 0.f || u + v > 1.f)
				return false;
			const float t = f * Vector3::Dot(edge2, q);
			
			if (t < ray.min || t > ray.max) return false;

			if (!ignoreHitRecord)
			{
				hitRecord.didHit = true;
				hitRecord.materialIndex = triangle.materialIndex;
				hitRecord.t = t;
				hitRecord.origin = ray.origin + ray.direction*t;
				hitRecord.normal = Vector3::Cross(edge1, edge2).Normalized();
			}

			return true;
			
#else
			const Vector3 edge1{ triangle.v1 - triangle.v0 };
			const Vector3 edge2{ triangle.v2 - triangle.v0 };
			const Vector3 n{ Vector3::Cross(edge1,edge2) };
			const float nRayDot{ Vector3::Dot(n, ray.direction) };
			if (nRayDot == 0) return false;
			if (!ignoreHitRecord)
			{
				if (triangle.cullMode == TriangleCullMode::FrontFaceCulling && nRayDot < 0) return false;
				if (triangle.cullMode == TriangleCullMode::BackFaceCulling && nRayDot > 0) return false;
			}
			else
			{
				if (triangle.cullMode == TriangleCullMode::FrontFaceCulling && nRayDot > 0) return false;
				if (triangle.cullMode == TriangleCullMode::BackFaceCulling && nRayDot < 0) return false;
			}
			const Vector3 L{ triangle.v0 - ray.origin };
			const float t{ Vector3::Dot(L,n) / Vector3::Dot(ray.direction,n) };
			if (t< ray.min || t > ray.max) return false;

			const Vector3 P{ ray.origin + ray.direction * t };
			
			Vector3 triVerts[3]{ triangle.v0,triangle.v1,triangle.v2 };
			for (size_t i = 0; i < 3; i++)
			{
				const Vector3 e{ triVerts[(i + 1) % 3] - triVerts[i] };
				const Vector3 p{P - triVerts[i] };
				if (Vector3::Dot(Vector3::Cross(e, p), n) < 0) return false;
			}

			if (!ignoreHitRecord)
			{
				hitRecord.didHit = true;
				hitRecord.materialIndex = triangle.materialIndex;
				hitRecord.t = t;
				hitRecord.origin = P;
				hitRecord.normal = Vector3::Cross(edge1, edge2).Normalized();
			}

			return true;
#endif
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			//assert(false && "No Implemented Yet!");
			Triangle triangle {};
			Vector3 vertA, vertB, vertC;
			HitRecord tempHit{};
			
			for (int i = 0; i < mesh.indices.size(); i+=3)
			{
				vertA = mesh.transformedPositions[mesh.indices[i ]];
				vertB = mesh.transformedPositions[mesh.indices[i + 1]];
				vertC = mesh.transformedPositions[mesh.indices[i + 2]];
				triangle = Triangle{ vertA,vertB,vertC };
				//triangle.normal = mesh.transformedNormals[i];
				triangle.cullMode = mesh.cullMode;
				if (!ignoreHitRecord)
				{
					if (HitTest_Triangle(triangle, ray, tempHit) && tempHit.t < hitRecord.t) hitRecord = tempHit;
				}
				else
				{
					if (HitTest_Triangle(triangle, ray)) return true;
				}
			}
			if (hitRecord.didHit) 
			{
				hitRecord.materialIndex = mesh.materialIndex;
				return true;
			}
			return false;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3& origin)
		{
			//todo W3
			//assert(false && "No Implemented Yet!");
			if (light.type == LightType::Point) return Vector3{ light.origin - origin };
			else return { light.direction * FLT_MAX };
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			//todo W3
			//assert(false && "No Implemented Yet!");
			if(light.type == LightType::Point)
			{
				Vector3 dist{ target - light.origin };
				//irradiance
				const ColorRGB Ergb{ light.color * (light.intensity / dist.SqrMagnitude()) };
				return Ergb;
			}
			else
			{
				const ColorRGB Ergb{light.color * light.intensity};
				return Ergb;
			}
		}

	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions,/* std::vector<Vector3>& normals,*/ std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof())
					break;
			}

			//Precompute normals
			/*for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if (isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}*/

			return true;
		}
#pragma warning(pop)
	}
}