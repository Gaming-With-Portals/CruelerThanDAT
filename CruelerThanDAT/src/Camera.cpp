#include "pch.hpp"
#include "Camera.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

CameraManager& CameraManager::Instance() {
	static CameraManager instance;
	return instance;
}

glm::mat4 CameraManager::getViewMatrix() {
	glm::mat4 rotMat = glm::toMat4(rotation);
	glm::vec3 transformedEye = glm::vec3(rotMat * glm::vec4(eye, 1.0));
	return glm::lookAt(transformedEye, center, up);
}

void CameraManager::Zoom(float amount) {
	glm::vec3 direction = glm::normalize(center - eye);
	float distance = glm::length(center - eye);
	distance = glm::clamp(distance - amount, 1.0f, 1000.0f);

	eye = center - direction * distance;
}

void CameraManager::SetWindowSize(int res_x, int res_y) {
	win_w = res_x;
	win_h = res_y;
}

void CameraManager::SetWindowPos(int pos_x, int pos_y) {
	win_x = pos_x;
	win_y = pos_y;
}

void CameraManager::Input(SDL_Event* e) {
	float x, y;
	SDL_GetMouseState(&x, &y);
	if (x < win_x && !dragging) {
		return;
	}

	if (e->type == SDL_EVENT_MOUSE_BUTTON_DOWN && e->button.button == SDL_BUTTON_LEFT) {
		dragging = true;
		v0 = arcballMap(e->button.x, e->button.y, win_x + win_w, win_y + win_h);
	}
	if (e->type == SDL_EVENT_MOUSE_BUTTON_UP && e->button.button == SDL_BUTTON_LEFT) {
		dragging = false;
	}
	if (e->type == SDL_EVENT_MOUSE_MOTION && dragging) {


		v1 = arcballMap(e->motion.x, e->motion.y, win_x + win_w, win_y + win_h);
		glm::vec3 axis = glm::cross(v0, v1);
		float angle = acosf(glm::min(1.0f, glm::dot(v0, v1)));

		if (glm::length(axis) > 0.0001f) {
			glm::quat delta = glm::angleAxis(angle, glm::normalize(axis));
			rotation = delta * rotation;
		}
		v0 = v1;
	}
	if (e->type == SDL_EVENT_MOUSE_WHEEL) {
		float zoomAmount = e->wheel.y * 1.0f; 
		Zoom(zoomAmount);
	}
}

glm::mat4 CameraManager::GetMatrixV() {
	glm::vec3 offset = rotation * glm::vec3(0, 0, glm::length(eye - center));
	glm::vec3 transformedEye = center + offset;
	return glm::lookAt(transformedEye, center, up);
}

glm::mat4 CameraManager::GetMatrixP() {
	return glm::perspective(glm::radians(60.0f), float(win_w) / float(win_h), 0.1f, 9000.0f);
}

glm::vec3 CameraManager::arcballMap(float x, float y, float w, float h) {
	glm::vec3 p;
	p.x = (2.0f * x - w) / w;
	p.y = (h - 2.0f * y) / h;
	p.z = 0.0f;

	float mag = p.x * p.x + p.y * p.y;
	if (mag <= 1.0f)
		p.z = sqrtf(1.0f - mag);
	else
		p = glm::normalize(p);
	return p;
}

CameraManager::CameraManager() {

}

CameraManager::~CameraManager() = default;
