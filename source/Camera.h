#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"
#include <iostream>
#include <algorithm>

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

		const float movementSpeed{ 5.0f };
		const float rotationSpeed{ 10.0f };

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
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			// keyboard movement
			if (pKeyboardState[SDL_SCANCODE_W])
			{
				origin += forward * movementSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_S])
			{
				origin -= forward * movementSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_D])
			{
				origin += right * movementSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_A])
			{
				origin -= right * movementSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_SPACE])
			{
				origin += up * movementSpeed * deltaTime;
			}
			if (pKeyboardState[SDL_SCANCODE_LSHIFT])
			{
				origin -= up * movementSpeed * deltaTime;
			}


			//mouse input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			// mouse movement & rotation
			if ((mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) && mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				const float upwards = -mouseY * movementSpeed * deltaTime;
				origin += up * upwards;
			}
			else if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT))
			{
				// not satisfied with speed at which the yaw was moving, added multiplier
				// to make it a bit faster
				const float multiplier{ 2.f };
				const float forwards = -mouseY * deltaTime;
				const float yaw = mouseX * multiplier * deltaTime;

				origin += forward * forwards;
				totalYaw += yaw;

				//calculate rotation matrix
				const Matrix finalRot = Matrix::CreateRotationX(totalPitch * TO_RADIANS) * Matrix::CreateRotationY(totalYaw * TO_RADIANS);
				forward = finalRot.TransformVector(Vector3::UnitZ);
				forward.Normalize();
			}
			else if (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				const float pitch = -mouseY * rotationSpeed * deltaTime;
				const float yaw = mouseX * rotationSpeed * deltaTime;

				totalPitch += pitch;
				totalYaw += yaw;

				totalPitch = std::clamp(totalPitch, -88.0f, 88.0f);

				if (totalYaw > 360.0f)
					totalYaw -= 360.0f;
				else if (totalYaw < 0.0f)
					totalYaw += 360.0f;

				// finally this works *cri* not straight CreateRotation but split in two
				const Matrix finalRot = Matrix::CreateRotationX(totalPitch * TO_RADIANS) * Matrix::CreateRotationY(totalYaw * TO_RADIANS);
				forward = finalRot.TransformVector(Vector3::UnitZ);
				forward.Normalize();
			}

			CalculateViewMatrix();
			CalculateProjectionMatrix();
		}

	};
}
