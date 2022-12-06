#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle) :
			origin{ _origin },
			fovAngle{ _fovAngle }
		{
			CalculateProjectionMatrix();
		}

		Vector3 origin{};
		float fovAngle{ 90.f };
		float fov{ tanf((fovAngle * TO_RADIANS) / 2.f) };
		float aspectRatio{};

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{};
		float totalYaw{};

		Matrix invViewMatrix{};
		Matrix viewMatrix{};
		Matrix projectionMatrix{};

		//CalcProjMatrix
		float nearVP{ 0.1f };
		float farVP{ 100.f };

		void Initialize(float _aspectRatio, float _fovAngle = 90.f, Vector3 _origin = { 0.f,0.f,0.f })
		{
			aspectRatio = _aspectRatio;
			fovAngle = _fovAngle;
			fov = tanf((fovAngle * TO_RADIANS) / 2.f);

			origin = _origin;
			forward = Vector3::UnitZ;
		}

		void CalculateViewMatrix()
		{
			//TODO W1

			//ViewMatrix => Matrix::CreateLookAtLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixlookatlh

			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right);

			invViewMatrix = Matrix{ right,up,forward,origin };
			viewMatrix = invViewMatrix.Inverse();
		}

		void CalculateProjectionMatrix()
		{
			//TODO W2

			/*xScale       0          0				   0
				0        yScale       0                0
				0          0       zf / (zf - zn)      1
				0          0      -zn * zf / (zf - zn) 0
				where:

			yScale = cot(fovY / 2)

			xScale = yScale / aspect ratio*/

			auto xScale = 1 / (aspectRatio * fov);
			auto yScale = 1 / fov;

			projectionMatrix = Matrix{ Vector4{ xScale, 0, 0, 0 },
										Vector4{ 0, yScale, 0, 0 },
										Vector4{ 0, 0 , farVP / (farVP - nearVP), 1},
										Vector4{ 0 , 0 ,-(nearVP * farVP) / (farVP - nearVP), 0} };

			//ProjectionMatrix => Matrix::CreatePerspectiveFovLH(...) [not implemented yet]
			//DirectX Implementation => https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixperspectivefovlh
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			const float movementSpeed = 8.f;
			const float mouseSens = 0.006f;

			DoKeyboardInput(deltaTime, movementSpeed);

			DoMouseInput(deltaTime, movementSpeed, mouseSens);

			Matrix finalRot{ Matrix::CreateRotation(totalPitch, totalYaw, 0) };

			forward = finalRot.TransformVector(Vector3::UnitZ);
			forward.Normalize();




			//Update Matrices
			CalculateViewMatrix();
			CalculateProjectionMatrix(); //Try to optimize this - should only be called once or when fov/aspectRatio changes
		}

		void DoKeyboardInput(float deltaTime, float moveSpeed)
		{

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin += moveSpeed * deltaTime * forward;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin -= moveSpeed * deltaTime * forward;
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin -= moveSpeed * deltaTime * right;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin += moveSpeed * deltaTime * right;
			}
			if (pKeyboardState[SDL_SCANCODE_Q])
			{
				origin.y -= moveSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_E])
			{
				origin.y += moveSpeed * deltaTime;
			}
		}

		void DoMouseInput(float deltaTime, float moveSpeed, float mouseSens)
		{
			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			if (mouseState & SDL_BUTTON_RMASK && mouseState & SDL_BUTTON_LMASK)
			{
				origin.y -= moveSpeed * deltaTime * mouseY;
			}
			else if (mouseState & SDL_BUTTON_RMASK)
			{
				totalYaw += mouseSens * mouseX;
				totalPitch -= mouseSens * mouseY;
				if (abs(totalPitch) >= float(M_PI) / 2.f)
				{
					if (totalPitch < 0)
					{
						totalPitch = -float(M_PI) / 2.001f;
					}
					else
					{
						totalPitch = float(M_PI) / 2.001f;
					}
				}
			}
			else if (mouseState & SDL_BUTTON_LMASK)
			{
				totalPitch -= mouseSens * mouseX;
				this->origin -= moveSpeed * deltaTime * this->forward * float(mouseY);
			}
		}
	};
}
