#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>

namespace dae
{
	Texture::Texture(SDL_Surface* pSurface) :
		m_pSurface{ pSurface },
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
	}

	Texture::~Texture()
	{
		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}
	}

	Texture* Texture::LoadFromFile(const std::string& path)
	{
		//TODO
		//Load SDL_Surface using IMG_LOAD
		//Create & Return a new Texture Object (using SDL_Surface)

		SDL_Surface* pSurface{ IMG_Load(path.c_str()) };
		Texture* newTexture{ new Texture(pSurface) };

		return newTexture;
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		//TODO
		//Sample the correct texel for the given uv

		auto u{ static_cast<int>(uv.x * m_pSurface->w) };
		auto v{ static_cast<int>(uv.y * m_pSurface->h) };
		
		uint8_t r{};
		uint8_t g{};
		uint8_t b{};

		auto pixel{ m_pSurfacePixels[u + v * m_pSurface->w] };
		SDL_GetRGB(pixel, m_pSurface->format, &r, &g, &b);
		

		// or maybe just divide each value by 255.f
		// but division is more expensive than multiplication
		// https://stackoverflow.com/questions/15745819/why-is-division-more-expensive-than-multiplication
		// (for refresher on why)
		const float conversion_to_01_range{ 1.f / 255.f };


		return ColorRGB{
			static_cast<float>(r) * conversion_to_01_range,
			static_cast<float>(g) * conversion_to_01_range,
			static_cast<float>(b) * conversion_to_01_range
		};
	}
}