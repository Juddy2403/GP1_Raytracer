//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"
#include <algorithm>
#include <execution>
#include <iostream>

using namespace dae;

bool Renderer::m_ShadowsEnabled = true;
Renderer::LightingMode Renderer::m_CurrentLightMode = LightingMode::Combined;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);

	m_ImageHorizontalIterator.resize(m_Width);
	m_ImageVerticalIterator.resize(m_Height);
	for (int index = 0; index < m_Width; ++index)
		m_ImageHorizontalIterator[index] = index;
	for (int index = 0; index < m_Height; ++index)
		m_ImageVerticalIterator[index] = index;
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();
	const float aspectRatio{ float(m_Width) / (m_Height) };
	Matrix cameraToWorld{ camera.CalculateCameraToWorld() };
	camera.UpdateFOV(45.f);

	std::for_each(std::execution::par, m_ImageVerticalIterator.begin(), m_ImageVerticalIterator.end(), [&](uint32_t py)
		//[this, aspectRatio, camera, materials, lights, cameraToWorld, pScene]
		{
			float cy = (1 - 2 * (py + 0.5f) / m_Height) * camera.fovFactor;
		
			std::for_each(std::execution::par, m_ImageHorizontalIterator.begin(), m_ImageHorizontalIterator.end(), [&](uint32_t px)
				//[this, aspectRatio, camera, materials, lights, cameraToWorld, pScene, cy, py]
				{
					Vector3 rayDirection{};
					float cx = (2.f * (px + 0.5f) / m_Width - 1.f) * aspectRatio * camera.fovFactor;
					rayDirection = Vector3{ cx, cy ,1 };
					rayDirection = cameraToWorld.TransformVector(rayDirection);
					rayDirection.Normalize();
					Ray hitRay{ camera.origin, rayDirection };
					ColorRGB finalColor{};
					HitRecord closestHit{};
					pScene->GetClosestHit(hitRay, closestHit);
					if (closestHit.didHit)
					{

						for (const Light& light : lights)
						{
							const Vector3 lightRayIntersectPoint{ closestHit.origin + 0.00001f * closestHit.normal };
							Vector3 lightRayDir{ LightUtils::GetDirectionToLight(light,lightRayIntersectPoint) };
							const float lightRayDist{ lightRayDir.Normalize() };

							const Ray lightRay{ lightRayIntersectPoint, lightRayDir, 0.001f, lightRayDist };

							const float lightDirCos{ Vector3::Dot(closestHit.normal,lightRayDir) };

							if (!pScene->DoesHit(lightRay) || !m_ShadowsEnabled)
							{
								switch (m_CurrentLightMode)
								{
								case LightingMode::Combined:
									if (lightDirCos >= 0)
										finalColor += LightUtils::GetRadiance(light, lightRayIntersectPoint) * lightDirCos
										* materials[closestHit.materialIndex]->Shade(closestHit, lightRayDir, -rayDirection);
									break;
								case LightingMode::ObservedArea:
									if (lightDirCos >= 0)
										finalColor += lightDirCos * ColorRGB{1, 1, 1};
									break;
								case LightingMode::Radiance:
									finalColor += LightUtils::GetRadiance(light, lightRayIntersectPoint);
									break;
								case LightingMode::BRDF:
									if (lightDirCos >= 0)
										finalColor += materials[closestHit.materialIndex]->Shade(closestHit, lightRayDir, -rayDirection);
									break;
								}
							}

						}
						//Update Color in Buffer
						finalColor.MaxToOne();
					}
					m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
						static_cast<uint8_t>(finalColor.r * 255),
						static_cast<uint8_t>(finalColor.g * 255),
						static_cast<uint8_t>(finalColor.b * 255));
				});
			
		});
	SDL_UpdateWindowSurface(m_pWindow);

#pragma region oldFor
	//float cx, cy;
	//for (int px{}; px < m_Width; ++px)
	//{
	//	cx = (2.f * (px + 0.5f) / m_Width - 1.f) * aspectRatio * camera.fovFactor;
	//	for (int py{}; py < m_Height; ++py)
	//	{
	//		Vector3 rayDirection{};
	//		cy = (1 - 2 * (py + 0.5f) / m_Height) * camera.fovFactor;
	//		rayDirection = Vector3{ cx, cy ,1 };
	//		rayDirection = cameraToWorld.TransformVector(rayDirection);

	//		rayDirection.Normalize();
	//		Ray hitRay{ camera.origin, rayDirection };

	//		ColorRGB finalColor{};
	//		HitRecord closestHit{};
	//		pScene->GetClosestHit(hitRay, closestHit);

	//		if (closestHit.didHit)
	//		{ 

	//			for (const Light& light : lights)
	//			{
	//				const Vector3 lightRayIntersectPoint{ closestHit.origin + 0.00001f * closestHit.normal };
	//				Vector3 lightRayDir{ LightUtils::GetDirectionToLight(light,lightRayIntersectPoint) };
	//				const float lightRayDist{ lightRayDir.Normalize() };

	//				const Ray lightRay{ lightRayIntersectPoint, lightRayDir, 0.001f, lightRayDist };

	//				const float lightDirCos{ Vector3::Dot(closestHit.normal,lightRayDir) };

	//				if (!pScene->DoesHit(lightRay) || !m_ShadowsEnabled )
	//				{
	//					switch (m_CurrentLightMode)
	//					{
	//					case LightingMode::Combined:
	//						if (lightDirCos >= 0)
	//						finalColor += LightUtils::GetRadiance(light, lightRayIntersectPoint) * lightDirCos
	//							* materials[closestHit.materialIndex]->Shade(closestHit, lightRayDir, -rayDirection);
	//						break;
	//					case LightingMode::ObservedArea:
	//						if (lightDirCos >= 0)
	//						finalColor += lightDirCos * ColorRGB{1, 1, 1};
	//						break;
	//					case LightingMode::Radiance:
	//						finalColor += LightUtils::GetRadiance(light, lightRayIntersectPoint);
	//						break;
	//					case LightingMode::BRDF:
	//						if (lightDirCos >= 0)
	//						finalColor += materials[closestHit.materialIndex]->Shade(closestHit, lightRayDir, -rayDirection);
	//						break;
	//					}
	//				}

	//			}
	//			//Update Color in Buffer
	//			finalColor.MaxToOne();
	//		}
	//		m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
	//			static_cast<uint8_t>(finalColor.r * 255),
	//			static_cast<uint8_t>(finalColor.g * 255),
	//			static_cast<uint8_t>(finalColor.b * 255));
	//	}
	//}
	////@END
	////Update SDL Surface
	//SDL_UpdateWindowSurface(m_pWindow);
}
#pragma endregion

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::ToggleShadow()
{
	m_ShadowsEnabled = !m_ShadowsEnabled;
}

void Renderer::ToggleLightMode()
{
	m_CurrentLightMode = LightingMode((int(m_CurrentLightMode) + 1) % 4);
}
