#pragma once

#include <cstdint>
#include <vector>

#include "Camera.h"
#include "DataTypes.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Texture;
	struct Mesh;
	struct Vertex;
	class Timer;
	class Scene;

	class Renderer final
	{
	public:

		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(Timer* pTimer);
		void Render();

		bool SaveBufferToImage() const;
		void SwitchVisualizationMethod();

	private:
		SDL_Window* m_pWindow{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};

		Camera m_Camera{};

		int m_Width{};
		int m_Height{};
		float m_AspectRatio{};
		Texture* m_pTexture{ nullptr };

		enum class VisualizationMethod
		{
			DepthBuffer,
			FinalColor
		};

		VisualizationMethod m_VisualizationMethod{ VisualizationMethod::FinalColor };

		void VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const;
		void VertexTransformationFunction(std::vector<Mesh>& meshes) const;
		void RenderTrianglesMesh(const Mesh& mesh, const std::vector<Vector2>& screenVertices, const std::vector<Vertex> ndcVertices, size_t vertIdx, bool swapVertices = false);
		//void ConvertFromNDCtoScreen();


		void RenderW6();
		void RenderW7();
	};
}
