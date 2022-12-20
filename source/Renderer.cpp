//External includes
#include "SDL.h"
#include "SDL_surface.h"
#include <iostream>

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Texture.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow),
	m_pTexture{}
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

	//Create Buffers
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	m_pDepthBufferPixels = new float[m_Width * m_Height];
	m_AspectRatio = static_cast<float>(m_Width) / m_Height;

	const int nrPixels{ m_Width * m_Height };
	m_pDepthBufferPixels = new float[nrPixels];
	std::fill_n(m_pDepthBufferPixels, nrPixels, FLT_MAX);

	//Initialize Camera
	m_Camera.Initialize(float(m_Width) / m_Height, 60.f, { 0.f, 5.f, -30.f });

	m_pTexture = m_pTexture->LoadFromFile("Resources/uv_grid_2.png");
}

Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
	m_pDepthBufferPixels = nullptr;
	delete m_pTexture;
	m_pTexture = nullptr;
}

void Renderer::Update(Timer* pTimer)
{
	m_Camera.Update(pTimer);
}

void Renderer::Render()
{
	//@START
	//Lock BackBuffer
	SDL_FillRect(m_pBackBuffer, nullptr, SDL_MapRGB(m_pBackBuffer->format, 100, 100, 100));
	const int nrPixels{ m_Width * m_Height };
	std::fill_n(m_pDepthBufferPixels, nrPixels, FLT_MAX);
	SDL_LockSurface(m_pBackBuffer);

	//RENDER LOGIC
	//for (int px{}; px < m_Width; ++px)
	//{
	//	for (int py{}; py < m_Height; ++py)
	//	{
	//		float gradient = px / static_cast<float>(m_Width);
	//		gradient += py / static_cast<float>(m_Width);
	//		gradient /= 2.0f;

	//		ColorRGB finalColor{ gradient, gradient, gradient };

	//		//Update Color in Buffer
	//		finalColor.MaxToOne();

	//		m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
	//			static_cast<uint8_t>(finalColor.r * 255),
	//			static_cast<uint8_t>(finalColor.g * 255),
	//			static_cast<uint8_t>(finalColor.b * 255));
	//	}
	//}

	//RenderW6();
	RenderW7();

	//@END
	//Update SDL Surface
	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::VertexTransformationFunction(const std::vector<Vertex>& vertices_in, std::vector<Vertex>& vertices_out) const
{
	//W6 Projection Stage

	vertices_out.clear();
	vertices_out.reserve(vertices_in.size());
	for (const auto& vert_in : vertices_in)
	{
		Vertex vert_out{};
		vert_out.position = m_Camera.invViewMatrix.TransformPoint(vert_in.position);

		vert_out.position.x = vert_out.position.x / vert_out.position.z / (m_Camera.fov * m_AspectRatio);
		vert_out.position.y = vert_out.position.y / vert_out.position.z / (m_Camera.fov);
		vertices_out.emplace_back(vert_out);
	}


}

void Renderer::VertexTransformationFunction(std::vector<Mesh>& meshes) const
{
	// W7 Projection

	for (auto& mesh : meshes)
	{
		mesh.vertices_out.clear();
		mesh.vertices_out.reserve(mesh.vertices.size());

		Matrix wvProjectionMatrix = mesh.worldMatrix * m_Camera.viewMatrix * m_Camera.projectionMatrix;

		for (auto const& vert_in : mesh.vertices)
		{
			// for every vert input in the mesh
			Vertex_Out vert_out{};


			/*vert_out.position = { m_Camera.invViewMatrix.TransformPoint(vert_in.position), 0 };

			vert_out.position.x = vert_out.position.x / vert_out.position.z / (m_Camera.fov * m_AspectRatio);
			vert_out.position.y = vert_out.position.y / vert_out.position.z / (m_Camera.fov);
			mesh.vertices_out.emplace_back(vert_out);*/

			// NDC space*****
			vert_out.position = wvProjectionMatrix.TransformPoint({ vert_in.position, 1.0f });

			vert_out.position.x /= vert_out.position.w;
			vert_out.position.y /= vert_out.position.w;
			vert_out.position.z /= vert_out.position.w;

			vert_out.color = vert_in.color;
			vert_out.normal = vert_in.normal;
			vert_out.uv = vert_in.uv;
			vert_out.tangent = vert_in.tangent;

			mesh.vertices_out.emplace_back(vert_out);
		}
	}

}



void dae::Renderer::RenderW6()
{
	std::vector<Vertex> verts_world
	{
		// First triangle 0
		Vertex{Vector3{0.f, 2.f, 0.f}, ColorRGB{1.f, 0.f, 0.f} },
		Vertex{Vector3{1.5f, -1.f, 0.f}, ColorRGB{1.f, 0.f, 0.f}},
		Vertex{Vector3{-1.5f, -1.f, 0.f}, ColorRGB{1.f, 0.f, 0.f}},

		// Second triangle 1
		Vertex{Vector3{0.f, 4.f, 2.f}, ColorRGB{1.f, 0.f, 0.f} },
		Vertex{Vector3{3.f, -2.f, 2.f}, ColorRGB{0.f, 1.f, 0.f}},
		Vertex{Vector3{-3.f, -2.f, 2.f}, ColorRGB{0.f, 0.f, 1.f}}

	};

	// Convert verts to NDC
	std::vector<Vertex> verts_ndc;
	VertexTransformationFunction(verts_world, verts_ndc);

	std::vector<Vector2> screenVerts;
	screenVerts.reserve(verts_ndc.size());
	for (const auto& vertexNdc : verts_ndc)
	{
		screenVerts.emplace_back(
			Vector2{
				(vertexNdc.position.x + 1) * 0.5f * static_cast<float>(m_Width),
				(1 - vertexNdc.position.y) * 0.5f * static_cast<float>(m_Height)
			}
		);
	}
	const Vector2 screenVector{ static_cast<float>(m_Width), static_cast<float>(m_Height) };
	for (size_t triIdx{}; triIdx < screenVerts.size(); triIdx += 3)
	{
		Vector2 boundingBoxMin{ Vector2::Min(screenVerts[triIdx], Vector2::Min(screenVerts[triIdx + 1], screenVerts[triIdx + 2])) };
		Vector2 boundingBoxMax{ Vector2::Max(screenVerts[triIdx], Vector2::Max(screenVerts[triIdx + 1], screenVerts[triIdx + 2])) };

		boundingBoxMin = Vector2::Max(Vector2::Zero, Vector2::Min(boundingBoxMin, screenVector));
		boundingBoxMax = Vector2::Max(Vector2::Zero, Vector2::Min(boundingBoxMax, screenVector));
		for (int px{ static_cast<int>(boundingBoxMin.x) }; px < boundingBoxMax.x; ++px)
		{
			for (int py{ static_cast<int>(boundingBoxMin.y) }; py < boundingBoxMax.y; ++py)
			{
				const int pixelIdx{ px + py * m_Width };
				const Vector2 pixelCoordinates{ static_cast<float>(px), static_cast<float>(py) };
				float signedAreaV0V1;
				float signedAreaV1V2;
				float signedAreaV2V0;

				if (GeometryUtils::IsPointInTriangle(screenVerts[triIdx], screenVerts[triIdx + 1],
					screenVerts[triIdx + 2], pixelCoordinates, signedAreaV0V1, signedAreaV1V2, signedAreaV2V0))
				{
					const float triangleArea{ 1.f / (Vector2::Cross(screenVerts[triIdx + 1] - screenVerts[triIdx],
						screenVerts[triIdx + 2] - screenVerts[triIdx])) };

					const float weightV0{ signedAreaV1V2 * triangleArea };
					const float weightV1{ signedAreaV2V0 * triangleArea };
					const float weightV2{ signedAreaV0V1 * triangleArea };

					const float depthWeight
					{
						(verts_world[triIdx].position.z - m_Camera.origin.z) * weightV0 +
						(verts_world[triIdx + 1].position.z - m_Camera.origin.z) * weightV1 +
						(verts_world[triIdx + 2].position.z - m_Camera.origin.z) * weightV2
					};

					if (m_pDepthBufferPixels[pixelIdx] < depthWeight) continue;
					m_pDepthBufferPixels[pixelIdx] = depthWeight;
					ColorRGB finalColor =
					{
						verts_world[triIdx].color * weightV0 +
						verts_world[triIdx + 1].color * weightV1 +
						verts_world[triIdx + 2].color * weightV2
					};


					//Update Color in Buffer
					finalColor.MaxToOne();

					m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
						static_cast<uint8_t>(finalColor.r * 255),
						static_cast<uint8_t>(finalColor.g * 255),
						static_cast<uint8_t>(finalColor.b * 255));
				}
			}
		}
	}
}

void dae::Renderer::RenderW7()
{
	// Define Mesh
	std::vector<Mesh> meshes_world
	{
		Mesh{
			{
				// Verts (not vert outs, those are still empty)
				Vertex{{-3, 3, -2}, colors::White, Vector2{0,0}},
				Vertex{{0, 3, -2}, colors::White, Vector2{.5f,0}},
				Vertex{{3, 3, -2}, colors::White, Vector2{1.f,0}},
				Vertex{{-3, 0, -2}, colors::White, Vector2{0,.5f}},
				Vertex{{0, 0, -2}, colors::White, Vector2{.5f,.5f}},
				Vertex{{3, 0, -2}, colors::White, Vector2{1.f,.5f}},
				Vertex{{-3, -3, -2}, colors::White, Vector2{0,1.f}},
				Vertex{{0, -3, -2}, colors::White, Vector2{.5f,1.f}},
				Vertex{{3, -3, -2}, colors::White, Vector2{1.f,1.f}}
				},
		{
			// Indices
			3,0,4,1,5,2,
			2,6,
			6,3,7,4,8,5
		},
		// Primitive topology
		PrimitiveTopology::TriangleStrip
		}
	};

	// Define Mesh
	//std::vector<Mesh> meshes_world
	//{
	//	Mesh{
	//		{
		//Vertex{ {-3, 3, -2}, colors::White, Vector2{0,0} },
		//Vertex{ {0, 3, -2}, colors::White, Vector2{.5f,0} },
		//Vertex{ {3, 3, -2}, colors::White, Vector2{1.f,0} },
		//Vertex{ {-3, 0, -2}, colors::White, Vector2{0,.5f} },
		//Vertex{ {0, 0, -2}, colors::White, Vector2{.5f,.5f} },
		//Vertex{ {3, 0, -2}, colors::White, Vector2{1.f,.5f} },
		//Vertex{ {-3, -3, -2}, colors::White, Vector2{0,1.f} },
		//Vertex{ {0, -3, -2}, colors::White, Vector2{.5f,1.f} },
		//Vertex{ {3, -3, -2}, colors::White, Vector2{1.f,1.f} }
	//			},
	//	{
	//		3,0,1,	1,4,3,	4,1,2,
	//		2,5,4,	6,3,4,	4,7,6,
	//		7,4,5,	5,8,7
	//	},
	//	PrimitiveTopology::TriangleList
	//	}
	//};

	// Changes the vert outs
	VertexTransformationFunction(meshes_world);

	for (auto& mesh : meshes_world)
	{
		//VertexTransformationFunction(mesh.vertices, vertices_ndc);

		//std::vector<Vertex> verts_ndc;
		//std::vector<Vector2> screen_verts;
		//
		//screen_verts.reserve(verts_ndc.size());
		//for (const auto& vert : verts_ndc)
		//{
		//	screen_verts.emplace_back(
		//		Vector2{
		//			// NDC
		//			(vert.position.x + 1) * 0.5f * m_Width,
		//			(1 - vert.position.y) * 0.5f * m_Height
		//
		//		}
		//	);
		//}

		for (auto& vert : mesh.vertices_out)
		{
			// for every vert out
			// that is projected via Vertextransformationfunction

			// convert from NDC to screen space
			vert = ConvertFromNDCtoScreen(vert);
		}

		switch (mesh.primitiveTopology)
		{
		case PrimitiveTopology::TriangleStrip:
			for (size_t vertIdx{}; vertIdx < mesh.indices.size() - 2; ++vertIdx)
			{
				RenderTrianglesMesh(mesh, mesh.vertices_out, mesh.vertices, vertIdx, vertIdx % 2);
			}
			break;
		case PrimitiveTopology::TriangleList:
			for (size_t vertIdx{}; vertIdx < mesh.indices.size() - 2; vertIdx += 3)
			{
				RenderTrianglesMesh(mesh, mesh.vertices_out, mesh.vertices, vertIdx);
			}
			break;
		}

		//for (int vertIdx{}; vertIdx < mesh.indices.size(); ++vertIdx)
		//{
		//	RenderTrianglesMesh(mesh, mesh.vertices_out, mesh.vertices, vertIdx);
		//}
		
	}

}

void dae::Renderer::RenderTrianglesMesh(const Mesh& mesh, const std::vector<Vertex_Out>& verts_out, const std::vector<Vertex> verts, size_t vertIdx, bool swapVerts)
{

	// Set up the proper indices for making the triangles based on the current idx
	auto vertIdx0{ mesh.indices[vertIdx + swapVerts * 2] };
	auto vertIdx1{ mesh.indices[vertIdx + 1] };
	auto vertIdx2{ mesh.indices[vertIdx + !swapVerts * 2] };

	//if (mesh.primitiveTopology == PrimitiveTopology::TriangleList)
	//{
	//	vertIdx0 = mesh.indices[vertIdx];
	//	vertIdx1 = mesh.indices[vertIdx + 1];
	//	vertIdx2 = mesh.indices[vertIdx + 2];
	//	vertIdx += 2;
	//}
	//else if (mesh.primitiveTopology == PrimitiveTopology::TriangleStrip)
	//{
	//	vertIdx0 = mesh.indices[vertIdx];

	//	if (vertIdx % 2 == 0)
	//	{
	//		vertIdx1 = mesh.indices[vertIdx + 1];
	//		// out of range****
	//		vertIdx2 = mesh.indices[vertIdx + 2];
	//	}
	//	else
	//	{
	//		vertIdx1 = mesh.indices[vertIdx + 2];
	//		vertIdx2 = mesh.indices[vertIdx + 1];
	//	}

	//	if (vertIdx + 3 >= mesh.indices.size())
	//		vertIdx += 2;
	//}

	if (vertIdx0 == vertIdx1 || vertIdx1 == vertIdx2 || vertIdx2 == vertIdx0) return;
	// Is in frustrum
	//if (!CheckIfIsInFrustrum(mesh.vertices_out[vertIdx0]) || !CheckIfIsInFrustrum(mesh.vertices_out[vertIdx1]) || !CheckIfIsInFrustrum(mesh.vertices_out[vertIdx2])) return;


	// make vert2s out of the vertexes for use with the code further on
	auto vert0 = mesh.vertices_out[vertIdx0];
	auto vert1 = mesh.vertices_out[vertIdx1];
	auto vert2 = mesh.vertices_out[vertIdx2];

	Vector2 posVert0{ vert0.position.GetXY() };
	Vector2 posVert1{ vert1.position.GetXY() };
	Vector2 posVert2{ vert2.position.GetXY() };

	// depths
	const float depthV0{ vert0.position.z };
	const float depthV1{ vert1.position.z };
	const float depthV2{ vert2.position.z };

	//const Vector2 v0{ Vertex1.position.GetXY() };
	//const Vector2 v1{ Vertex2.position.GetXY() };
	//const Vector2 v2{ Vertex3.position.GetXY() };

	//const float wV0{ Vertex1.position.w };
	//const float wV1{ Vertex2.position.w };
	//const float wV2{ Vertex3.position.w };

	//const Vector2 v0uv{ Vertex1.uv };
	//const Vector2 v1uv{ Vertex2.uv };
	//const Vector2 v2uv{ Vertex3.uv };

	//const Vector2 edge01{ v1 - v0 };
	//const Vector2 edge12{ v2 - v1 };
	//const Vector2 edge20{ v0 - v2 };

	// Setting up bounding box
	Vector2 boundingBoxMin{ Vector2::Min(posVert0, Vector2::Min(posVert1, posVert2))};
	Vector2 boundingBoxMax{ Vector2::Max(posVert0, Vector2::Max(posVert1, posVert2)) };

	const Vector2 screenVector{ static_cast<float>(m_Width), static_cast<float>(m_Height) };

	// Setting up bounding box in NDC
	boundingBoxMin = Vector2::Max(Vector2::Zero, Vector2::Min(boundingBoxMin, screenVector));
	boundingBoxMax = Vector2::Max(Vector2::Zero, Vector2::Min(boundingBoxMax, screenVector));

	const int bbBottom = std::min(static_cast<int>(std::min(posVert0.y, posVert1.y)), static_cast<int>(posVert2.y));
	const int bbTop = std::max(static_cast<int>(std::max(posVert0.y, posVert1.y)), static_cast<int>(posVert2.y)) + 1;

	const int bbLeft = std::min(static_cast<int>(std::min(posVert0.x, posVert1.x)), static_cast<int>(posVert2.x));
	const int bbRight = std::max(static_cast<int>(std::max(posVert0.x, posVert1.x)), static_cast<int>(posVert2.x)) + 1;

	// Is bb in Screen?
	if (bbLeft <= 0 || bbRight >= m_Width - 1)
		return;

	if (bbBottom <= 0 || bbTop >= m_Height - 1)
		return;

	const int offSet{ 1 };

	//for (int px{ static_cast<int>(boundingBoxMin.x) }; px < boundingBoxMax.x; ++px)
	//{
	//	for (int py{ static_cast<int>(boundingBoxMin.y) }; py < boundingBoxMax.y; ++py)
	for (int px{ bbLeft - offSet }; px < bbRight + offSet; ++px)
		{
		for (int py{ bbBottom - offSet }; py < bbTop + offSet; ++py)
		{
			// Final color variable
			ColorRGB finalColor{ colors::Black };

			const int pixelIdx{ px + py * m_Width };
			const Vector2 pixelCoordinates{ static_cast<float>(px), static_cast<float>(py) };
			
			float signedAreaVert0_1{}, signedAreaVert1_2{}, signedAreaVert2_0{};

			if (GeometryUtils::IsPointInTriangle(posVert0, posVert1,
				posVert2, pixelCoordinates, signedAreaVert0_1, signedAreaVert1_2, signedAreaVert2_0))
			{
				const float triangleArea{ 1.f / (Vector2::Cross(posVert1 - posVert0,
					posVert2 - posVert0)) };

				const float weightV0{ signedAreaVert1_2 * triangleArea };
				const float weightV1{ signedAreaVert2_0 * triangleArea };
				const float weightV2{ signedAreaVert0_1 * triangleArea };


				// switch for changing between with or without depth buffer
				switch (m_VisualizationMethod)
				{
				case VisualizationMethod::FinalColor:
				{
					//sampling the UV coordinates and color
					const float depthInterpolated
					{
						1.f / ((1.f / depthV0) * weightV0 +
						(1.f / depthV1) * weightV1 +
						(1.f / depthV2) * weightV2)
					};

					if (m_pDepthBufferPixels[pixelIdx] < depthInterpolated) continue;
					m_pDepthBufferPixels[pixelIdx] = depthInterpolated;

					Vector2 pixelUV
					{
						(mesh.vertices[vertIdx0].uv / depthV0 * weightV0 +
						mesh.vertices[vertIdx1].uv / depthV1 * weightV1 +
						mesh.vertices[vertIdx2].uv / depthV2 * weightV2) * depthInterpolated
					};

					finalColor = m_pTexture->Sample(pixelUV);
					break;
				}
				case VisualizationMethod::DepthBuffer:
				{
					const float depthRemapSize{ 0.005f };

					//float remapedBufferVal{ ZBufferVal };
					//DepthRemap(remapedBufferVal, depthRemapSize);
					//finalColor = ColorRGB{ remapedBufferVal, remapedBufferVal , remapedBufferVal };
					break;
				}
				}

				//Update Color in Buffer
				finalColor.MaxToOne();

				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
			}
		}
	}


}

void dae::Renderer::RenderTrianglesMesh(const Mesh& mesh, const std::vector<Vector2>& screenVertices, const std::vector<Vertex> ndcVertices, size_t vertIdx, bool swapVertices)
{
	auto vertIdx0{ mesh.indices[vertIdx + swapVertices * 2] };
	auto vertIdx1{ mesh.indices[vertIdx + 1] };
	auto vertIdx2{ mesh.indices[vertIdx + !swapVertices * 2] };
	
	if (vertIdx0 == vertIdx1 || vertIdx1 == vertIdx2 || vertIdx2 == vertIdx0) return;

	// Setting up bounding box
	Vector2 boundingBoxMin{ Vector2::Min(screenVertices[vertIdx0], Vector2::Min(screenVertices[vertIdx1], screenVertices[vertIdx2])) };
	Vector2 boundingBoxMax{ Vector2::Max(screenVertices[vertIdx0], Vector2::Max(screenVertices[vertIdx1], screenVertices[vertIdx2])) };

	const Vector2 screenVector{ static_cast<float>(m_Width), static_cast<float>(m_Height) };

	// Setting up bounding box in NDC
	boundingBoxMin = Vector2::Max(Vector2::Zero, Vector2::Min(boundingBoxMin, screenVector));
	boundingBoxMax = Vector2::Max(Vector2::Zero, Vector2::Min(boundingBoxMax, screenVector));



	for (int px{ static_cast<int>(boundingBoxMin.x) }; px < boundingBoxMax.x; ++px)
	{
		for (int py{ static_cast<int>(boundingBoxMin.y) }; py < boundingBoxMax.y; ++py)
		{
			// Final color variable
			ColorRGB finalColor{ colors::Black };


			const int pixelIdx{ px + py * m_Width };
			const Vector2 pixelCoordinates{ static_cast<float>(px), static_cast<float>(py) };
			float signedAreaV0V1{}, signedAreaV1V2{}, signedAreaV2V0{};

			if (GeometryUtils::IsPointInTriangle(screenVertices[vertIdx0], screenVertices[vertIdx1],
				screenVertices[vertIdx2], pixelCoordinates, signedAreaV0V1, signedAreaV1V2, signedAreaV2V0))
			{
				const float triangleArea{ 1.f / (Vector2::Cross(screenVertices[vertIdx1] - screenVertices[vertIdx0],
					screenVertices[vertIdx2] - screenVertices[vertIdx0])) };

				const float weightV0{ signedAreaV1V2 * triangleArea };
				const float weightV1{ signedAreaV2V0 * triangleArea };
				const float weightV2{ signedAreaV0V1 * triangleArea };

				const float depthV0{ ndcVertices[vertIdx0].position.z };
				const float depthV1{ ndcVertices[vertIdx1].position.z };
				const float depthV2{ ndcVertices[vertIdx2].position.z };

					// switch for changing between with or without depth buffer
					switch (m_VisualizationMethod)
					{
					case VisualizationMethod::FinalColor:
					{
						//sampling the UV coordinates and color
						const float depthInterpolated
						{
							1.f / ((1.f / depthV0) * weightV0 +
							(1.f / depthV1) * weightV1 +
							(1.f / depthV2) * weightV2)
						};

						if (m_pDepthBufferPixels[pixelIdx] < depthInterpolated) continue;
						m_pDepthBufferPixels[pixelIdx] = depthInterpolated;

						Vector2 pixelUV
						{
							(mesh.vertices[vertIdx0].uv / depthV0 * weightV0 +
							mesh.vertices[vertIdx1].uv / depthV1 * weightV1 +
							mesh.vertices[vertIdx2].uv / depthV2 * weightV2) * depthInterpolated
						};

						finalColor = m_pTexture->Sample(pixelUV);
						break;
					}
					case VisualizationMethod::DepthBuffer:
					{
						const float depthRemapSize{ 0.005f };

						//float remapedBufferVal{ ZBufferVal };
						//DepthRemap(remapedBufferVal, depthRemapSize);
						//finalColor = ColorRGB{ remapedBufferVal, remapedBufferVal , remapedBufferVal };
						break;
					}
					}

				//Update Color in Buffer
				finalColor.MaxToOne();

				m_pBackBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
			}
		}
	}


}

Vertex_Out dae::Renderer::ConvertFromNDCtoScreen(const Vertex_Out& vert_out)
{
	Vertex_Out vertex{ vert_out };

	// Doesn't change the original vertex, just returns the conversion from NDC to Screen space
	
	vertex.position.x = (vert_out.position.x + 1) * 0.5f * m_Width;
	vertex.position.y = (1 - vert_out.position.y) * 0.5f * m_Height;
	return vertex;
}

Vertex dae::Renderer::ConvertFromDNCtoScreen(const Vertex& vert)
{
	Vertex vertex{ vert };

	// Doesn't change the original vertex, just returns the conversion from NDC to Screen space

	vertex.position.x = (vert.position.x + 1) * 0.5f * m_Width;
	vertex.position.y = (1 - vert.position.y) * 0.5f * m_Height;
	return vertex;
}

bool dae::Renderer::CheckIfIsInFrustrum(const Vertex_Out& vert)
{

	if (vert.position.x < -1 || vert.position.x > 1)
		return false;
	else if (vert.position.y < -1 || vert.position.y > 1)
		return  false;
	else if (vert.position.z < 0 || vert.position.z > 1)
		return  false;

	return true;
}

void dae::Renderer::SwitchVisualizationMethod()
{
	switch (m_VisualizationMethod)
	{
	case VisualizationMethod::FinalColor:
	{
		m_VisualizationMethod = VisualizationMethod::DepthBuffer;
		break;
	}
	case VisualizationMethod::DepthBuffer:
	{
		m_VisualizationMethod = VisualizationMethod::FinalColor;
		break;
	}
	default:
		break;
	}
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBackBuffer, "Rasterizer_ColorBuffer.bmp");
}
