//#define _USE_MATH_DEFINES
//#include <cmath> 
//#include <iostream>
//#include <glm/gtc/matrix_transform.hpp>
//#include "Camera.h"
//#include "MatrixStack.h"
//
//Camera::Camera() :
//	aspect(1.0f),
//	fovy((float)(45.0*M_PI/180.0)),
//	znear(0.1f),
//	zfar(1000.0f),
//	rotations(0.0, 0.0),
//	translations(0.0f, 0.0f, -5.0f),
//	rfactor(0.01f),
//	tfactor(0.001f),
//	sfactor(0.005f)
//{
//}
//
//Camera::~Camera()
//{
//}
//
//void Camera::mouseClicked(float x, float y, bool shift, bool ctrl, bool alt)
//{
//	mousePrev.x = x;
//	mousePrev.y = y;
//	if(shift) {
//		state = Camera::TRANSLATE;
//	} else if(ctrl) {
//		state = Camera::SCALE;
//	} else {
//		state = Camera::ROTATE;
//	}
//}
//
//void Camera::mouseMoved(float x, float y)
//{
//	glm::vec2 mouseCurr(x, y);
//	glm::vec2 dv = mouseCurr - mousePrev;
//	switch(state) {
//		case Camera::ROTATE:
//			rotations += rfactor * dv;
//			break;
//		case Camera::TRANSLATE:
//			translations.x -= translations.z * tfactor * dv.x;
//			translations.y += translations.z * tfactor * dv.y;
//			break;
//		case Camera::SCALE:
//			translations.z *= (1.0f - sfactor * dv.y);
//			break;
//	}
//	mousePrev = mouseCurr;
//}
//
//void Camera::applyProjectionMatrix(std::shared_ptr<MatrixStack> P) const
//{
//	// Modify provided MatrixStack
//	P->multMatrix(glm::perspective(fovy, aspect, znear, zfar));
//}
//
//void Camera::applyViewMatrix(std::shared_ptr<MatrixStack> MV) const
//{
//	MV->translate(translations);
//	MV->rotate(rotations.y, glm::vec3(1.0f, 0.0f, 0.0f));
//	MV->rotate(rotations.x, glm::vec3(0.0f, 1.0f, 0.0f));
//}
#define _USE_MATH_DEFINES
#include <cmath> 
#include <iostream>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"
#include "MatrixStack.h"

using namespace std;

Camera::Camera() :
	aspect(1.0f),
	fovy((float)(45.0 * M_PI / 180.0)),
	znear(0.1f),
	zfar(1000.0f),
	pos(0.0f, 2.0f, 0.0f), // Start at head height
	yaw(0.0f),
	pitch(0.0f),
	rfactor(0.005f) // Mouse sensitivity
{
}

Camera::~Camera() {}

void Camera::mouseMoved(float x, float y) {
	glm::vec2 mouseCurr(x, y);
	glm::vec2 dv = mouseCurr - mousePrev;

	// Mouse X tied to Yaw, Mouse Y tied to Pitch
	yaw += rfactor * dv.x;
	pitch -= rfactor * dv.y;

	// Cap pitch between -60 and +60 degrees (approx 1.0 radians)
	pitch = std::max(-1.0f, std::min(1.0f, pitch));

	mousePrev = mouseCurr;
}

void Camera::updateWASD(bool w, bool a, bool s, bool d) {
	float speed = 0.2f;
	float worldHalfSize = 74.0f; // Slightly less than 75 to stay safely inside

	// 1. Calculate movement (Horizontal only)
	glm::vec3 moveForward(sin(yaw), 0.0f, cos(yaw));
	glm::vec3 right = glm::cross(moveForward, glm::vec3(0, 1, 0));

	// 2. Apply movement
	if (w) pos += moveForward * speed;
	if (s) pos -= moveForward * speed;
	if (a) pos -= right * speed;
	if (d) pos += right * speed;

	// 3. THE "STAY ON GROUND" FIXES:

	// Height Lock: Prevents flying/sinking
	pos.y = 2.0f;

	// Edge Lock: Prevents walking off the 150x150 square
	// This clamps your X and Z between -74 and +74
	pos.x = std::max(-worldHalfSize, std::min(worldHalfSize, pos.x));
	pos.z = std::max(-worldHalfSize, std::min(worldHalfSize, pos.z));
}

// Custom function for Zoom (z/Z keys)
void Camera::updateZoom(bool zoomIn, bool zoomOut) {
	float zoomSpeed = 0.02f;
	if (zoomIn)  fovy -= zoomSpeed;
	if (zoomOut) fovy += zoomSpeed;

	// Cap FOV between 4 and 114 degrees (in radians)
	fovy = std::max((float)(4.0 * M_PI / 180.0), std::min((float)(114.0 * M_PI / 180.0), fovy));
}

void Camera::applyProjectionMatrix(std::shared_ptr<MatrixStack> P) const {
	P->multMatrix(glm::perspective(fovy, aspect, znear, zfar));
}

void Camera::applyViewMatrix(std::shared_ptr<MatrixStack> MV) const {
	// Calculate look direction including pitch
	glm::vec3 lookDir;
	lookDir.x = cos(pitch) * sin(yaw);
	lookDir.y = sin(pitch);
	lookDir.z = cos(pitch) * cos(yaw);

	glm::vec3 eye = pos;
	glm::vec3 target = pos + lookDir;
	glm::vec3 up(0, 1, 0);

	MV->multMatrix(glm::lookAt(eye, target, up));
}

// Keep this to initialize mouse position when first clicked
void Camera::mouseClicked(float x, float y, bool shift, bool ctrl, bool alt) {
	mousePrev.x = x;
	mousePrev.y = y;
}