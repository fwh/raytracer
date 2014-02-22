
#ifndef CAMERA_H_SDFH987D
#define CAMERA_H_SDFH987D
#include "triple.h"
#include "scene.h"
#include <iostream>

class Camera
{
public:
	Point m_Eye;
	Point m_Center;
	Vector m_Up;
	int m_Width, m_Heigth;

	Vector direction() {
		return m_Center - m_Eye;
	}

	Vector right() {
		return direction().normalized().cross(m_Up);
	}
};
#endif /* end of include guard: CAMERA_H_SDFH987D */