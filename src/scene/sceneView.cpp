#pragma once
#include "platform/utils.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "scene/sceneView.h"
#include "platform/inputHandler.h"

SceneViewInputAdapter::SceneViewInputAdapter(SceneView* sceneView)
	: sceneView(sceneView)
{

}

void SceneViewInputAdapter::forward(bool triggered)
{
	sceneView->keys.up = triggered;
}

void SceneViewInputAdapter::backward(bool triggered)
{
	sceneView->keys.down = triggered;
}
void SceneViewInputAdapter::left(bool triggered)
{
	sceneView->keys.left = triggered;
}
void SceneViewInputAdapter::right(bool triggered)
{
	sceneView->keys.right = triggered;
}

void SceneViewInputAdapter::mouseLButton(bool triggered, float x, float y)
{
	mouseButtons.left = triggered;
	mousePos = glm::vec2(x, y);
}

void SceneViewInputAdapter::mouseRButton(bool triggered, float x, float y)
{
	mouseButtons.right = triggered;
	mousePos = glm::vec2(x, y);
}

void SceneViewInputAdapter::mouseMButton(bool triggered, float x, float y)
{
	mouseButtons.middle = triggered;
	mousePos = glm::vec2(x, y);
}

void SceneViewInputAdapter::mouseMove(float x, float y)
{
	int32_t dx = (int32_t) mousePos.x - x;
	int32_t dy = (int32_t) mousePos.y - y;

	bool handled = false;

	if (mouseButtons.left) {
		sceneView->rotate(glm::vec3(dy * sceneView->rotationSpeed, -dx * sceneView->rotationSpeed, 0.0f));
	}
	if (mouseButtons.right) {
		sceneView->translate(glm::vec3(-0.0f, 0.0f, dy * .005f));
	}
	if (mouseButtons.middle) {
		sceneView->translate(glm::vec3(-dx * 0.01f, -dy * 0.01f, 0.0f));
	}
	mousePos = glm::vec2((float)x, (float)y);
}

void SceneView::updateViewMatrix()
{
	glm::mat4 rotM = glm::mat4(1.0f);
	glm::mat4 transM;

	rotM = glm::rotate(rotM, glm::radians(rotation.x * (flipY ? -1.0f : 1.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	glm::vec3 translation = position;
	
	if (flipY)
	{
		translation.y *= -1.0f;
	}
	transM = glm::translate(glm::mat4(1.0f), translation);

	if (type == CameraType::firstperson)
	{
		matrices.view = rotM * transM;
	}
	else
	{
		matrices.view = transM * rotM;
	}

	viewPos = glm::vec4(position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

	updated = true;
};

SceneView::SceneView()
	: sceneViewInputAdapter(nullptr)
{
	sceneViewInputAdapter = new SceneViewInputAdapter(this);
}

SceneView::~SceneView()
{
	delete sceneViewInputAdapter;
}

SceneViewInputAdapter* SceneView::getSceneViewInputAdapter()
{
	return sceneViewInputAdapter;
}

bool SceneView::moving()
{
	return keys.left || keys.right || keys.up || keys.down;
}

float SceneView::getNearClip() {
	return znear;
}

float SceneView::getFarClip() {
	return zfar;
}

void SceneView::setPerspective(float fov, float aspect, float znear, float zfar)
{
	this->fov = fov;
	this->znear = znear;
	this->zfar = zfar;
	matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
	if (flipY) {
		matrices.perspective[1][1] *= -1.0f;
	}
};

void SceneView::updateAspectRatio(float aspect)
{
	matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
	if (flipY) {
		matrices.perspective[1][1] *= -1.0f;
	}
}

void SceneView::setPosition(glm::vec3 position)
{
	this->position = position;
	updateViewMatrix();
}

void SceneView::setRotation(glm::vec3 rotation)
{
	this->rotation = rotation;
	updateViewMatrix();
}

void SceneView::rotate(glm::vec3 delta)
{
	this->rotation += delta;
	updateViewMatrix();
}

void SceneView::setTranslation(glm::vec3 translation)
{
	this->position = translation;
	updateViewMatrix();
};

void SceneView::translate(glm::vec3 delta)
{
	this->position += delta;
	updateViewMatrix();
}

void SceneView::setRotationSpeed(float rotationSpeed)
{
	this->rotationSpeed = rotationSpeed;
}

void SceneView::setMovementSpeed(float movementSpeed)
{
	this->movementSpeed = movementSpeed;
}

void SceneView::update(float deltaTime)
{
	updated = false;
	if (type == CameraType::firstperson)
	{
		if (moving())
		{
			glm::vec3 camFront;
			camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
			camFront.y = sin(glm::radians(rotation.x));
			camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
			camFront = glm::normalize(camFront);

			float moveSpeed = 0.01 * movementSpeed;

			if (keys.up)
				position += camFront * moveSpeed;
			if (keys.down)
				position -= camFront * moveSpeed;
			if (keys.left)
				position -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;
			if (keys.right)
				position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * moveSpeed;

			updateViewMatrix();
		}
	}
};

// Update camera passing separate axis data (gamepad)
// Returns true if view or position has been changed
bool SceneView::updatePad(glm::vec2 axisLeft, glm::vec2 axisRight, float deltaTime)
{
	bool retVal = false;

	if (type == CameraType::firstperson)
	{
		// Use the common console thumbstick layout		
		// Left = view, right = move

		const float deadZone = 0.0015f;
		const float range = 1.0f - deadZone;

		glm::vec3 camFront;
		camFront.x = -cos(glm::radians(rotation.x)) * sin(glm::radians(rotation.y));
		camFront.y = sin(glm::radians(rotation.x));
		camFront.z = cos(glm::radians(rotation.x)) * cos(glm::radians(rotation.y));
		camFront = glm::normalize(camFront);

		float moveSpeed = deltaTime * movementSpeed * 2.0f;
		float rotSpeed = deltaTime * rotationSpeed * 50.0f;

		// Move
		if (fabsf(axisLeft.y) > deadZone)
		{
			float pos = (fabsf(axisLeft.y) - deadZone) / range;
			position -= camFront * pos * ((axisLeft.y < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
			retVal = true;
		}
		if (fabsf(axisLeft.x) > deadZone)
		{
			float pos = (fabsf(axisLeft.x) - deadZone) / range;
			position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * pos * ((axisLeft.x < 0.0f) ? -1.0f : 1.0f) * moveSpeed;
			retVal = true;
		}

		// Rotate
		if (fabsf(axisRight.x) > deadZone)
		{
			float pos = (fabsf(axisRight.x) - deadZone) / range;
			rotation.y += pos * ((axisRight.x < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
			retVal = true;
		}
		if (fabsf(axisRight.y) > deadZone)
		{
			float pos = (fabsf(axisRight.y) - deadZone) / range;
			rotation.x -= pos * ((axisRight.y < 0.0f) ? -1.0f : 1.0f) * rotSpeed;
			retVal = true;
		}
	}
	else
	{
		// todo: move code from example base class for look-at
	}

	if (retVal)
	{
		updateViewMatrix();
	}

	return retVal;
}