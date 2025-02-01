#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "Math.h"
#include "Timer.h"
#include "iostream"

namespace dae
{
	struct Camera
	{
		Camera() = default;

		Camera(const Vector3& _origin, float _fovAngle) :
			origin{ _origin },
			fovAngle{ _fovAngle }
		{
		}

		Vector3 origin{};
		float fovAngle{ 90.f };
		float fovFactor{ 1.f };

		Vector3 forward{ Vector3::UnitZ }; //0.266f,-0.453f,0.860f //Vector3::UnitZ
		Vector3 up{ Vector3::UnitY };
		Vector3 right{ Vector3::UnitX };

		float totalPitch{ 0.f };
		float totalYaw{ 0.f };

		Matrix cameraToWorld{};

		void UpdateFOV(float fovValue)
		{
			fovAngle = fovValue;
			fovFactor = tan(TO_RADIANS * fovValue / 2.f);
		}

		Matrix CalculateCameraToWorld()
		{
			//todo: W2

			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();

			return {
				Vector4 {right, 0},
				Vector4 {up, 0},
				Vector4 {forward, 0},
				Vector4 {origin, 1}
			};
			//return ONB;
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			//todo: W2
			//assert(false && "Not Implemented Yet");
			const float movementSpeed{ 20.f }, rotationSpeed{ 60.f };

			if (pKeyboardState[SDL_SCANCODE_W]) origin += (movementSpeed * deltaTime) * forward.Normalized();
			if (pKeyboardState[SDL_SCANCODE_S]) origin -= (movementSpeed * deltaTime) * forward.Normalized();
			if (pKeyboardState[SDL_SCANCODE_A]) origin -= (movementSpeed * deltaTime) * right.Normalized();
			if (pKeyboardState[SDL_SCANCODE_D]) origin += (movementSpeed * deltaTime) * right.Normalized();
			
			if (mouseState == SDL_BUTTON_LEFT && mouseY)	origin -= (movementSpeed * deltaTime) * forward.Normalized() * mouseY;
			if (mouseState == SDL_BUTTON_X2 && mouseY) origin -= (movementSpeed * deltaTime) * up.Normalized() * mouseY;
			if (mouseState == SDL_BUTTON_LEFT && mouseX) totalYaw += rotationSpeed * deltaTime * mouseX;

			if (mouseState == SDL_BUTTON_X1)
			{
				if (!(totalPitch > 88.f && mouseY <= 0) && !(totalPitch < -88.f && mouseY >= 0))
					totalPitch -= rotationSpeed * deltaTime * mouseY;

				totalYaw += rotationSpeed * deltaTime * mouseX;
			}
			Matrix finalRotation{};
			
			finalRotation = Matrix::CreateRotationX(totalPitch * TO_RADIANS);
			finalRotation *= Matrix::CreateRotationY(totalYaw * TO_RADIANS);

			forward = finalRotation.TransformVector(Vector3::UnitZ);
			forward.Normalize();
		}
	};
}
