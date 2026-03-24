//#pragma  once
//#ifndef CAMERA_H
//#define CAMERA_H
//
//#include <memory>
//
//#define GLM_FORCE_RADIANS
//#include <glm/glm.hpp>
//
//class MatrixStack;
//
//class Camera
//{
//public:
//	enum {
//		ROTATE = 0,
//		TRANSLATE,
//		SCALE
//	};
//	
//	Camera();
//	virtual ~Camera();
//	void setInitDistance(float z) { translations.z = -std::abs(z); }
//	void setAspect(float a) { aspect = a; };
//	void setRotationFactor(float f) { rfactor = f; };
//	void setTranslationFactor(float f) { tfactor = f; };
//	void setScaleFactor(float f) { sfactor = f; };
//	void mouseClicked(float x, float y, bool shift, bool ctrl, bool alt);
//	void mouseMoved(float x, float y);
//	void applyProjectionMatrix(std::shared_ptr<MatrixStack> P) const;
//	void applyViewMatrix(std::shared_ptr<MatrixStack> MV) const;
//	
//private:
//	float aspect;
//	float fovy;
//	float znear;
//	float zfar;
//	glm::vec2 rotations;
//	glm::vec3 translations;
//	glm::vec2 mousePrev;
//	int state;
//	float rfactor;
//	float tfactor;
//	float sfactor;
//};
//
//#endif


#pragma once
#ifndef CAMERA_H
#define CAMERA_H

#include <memory>
#include <algorithm> // Added for std::max/min if needed later

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

// Forward declaration
class MatrixStack;

class Camera
{
public:
	Camera();
	virtual ~Camera();

	// Task 2: Movement and Zoom Interface
	void updateWASD(bool w, bool a, bool s, bool d);
	void updateZoom(bool zoomIn, bool zoomOut);

	void setAspect(float a) { aspect = a; }
	void mouseClicked(float x, float y, bool shift, bool ctrl, bool alt);
	void mouseMoved(float x, float y);

	void applyProjectionMatrix(std::shared_ptr<MatrixStack> P) const;
	void applyViewMatrix(std::shared_ptr<MatrixStack> MV) const;

	// Task 5 Getters
	glm::vec3 getPos() const { return pos; }
	float getYaw() const { return yaw; }
	float getFOV() const { return fovy; }


private:
	float aspect;
	float fovy;
	float znear;
	float zfar;

	// Task 2: Freelook State
	glm::vec3 pos;   // Camera position (eye)
	float yaw;       // Horizontal angle (theta)
	float pitch;     // Vertical angle (phi)

	glm::vec2 mousePrev;
	float rfactor;   // Sensitivity
};

#endif