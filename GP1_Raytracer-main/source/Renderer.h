#pragma once

#include <cstdint>
#include "Utils.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer() = default;

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Scene* pScene) const;
		bool SaveBufferToImage() const;

		static void ToggleShadow();
		static void ToggleLightMode();
	private:

		enum class LightingMode
		{
			ObservedArea, //Lambert cosine law
			Radiance, //Incident Radiance
			BRDF, //Scattering of light
			Combined
		};

		static LightingMode m_CurrentLightMode;
		static bool m_ShadowsEnabled;

		SDL_Window* m_pWindow{};

		SDL_Surface* m_pBuffer{};
		uint32_t* m_pBufferPixels{};
		std::vector<uint32_t> m_ImageHorizontalIterator, m_ImageVerticalIterator;
		int m_Width{};
		int m_Height{};
	};
}
