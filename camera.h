#include "triple.h"
#include "scene.h"
#include <iostream>

#ifndef CAMERA_H_SDFH987D
#define CAMERA_H_SDFH987D
class Camera
{
public:
	Camera(Vector a_Eye) : m_Up(Triple(0.0,1.0,0.0)), m_Center(Triple(200, 200, 0))
	{
		std::cout << "0" << std::endl;
		m_Eye = a_Eye;
		Init();
	}

	Camera(Vector a_Eye, Vector a_up, Vector a_center) : m_Up(a_up), m_Center(a_center)
	{
		std::cout << "1" <<std::endl;
		m_Eye = a_Eye;
		Init();
	}
	~Camera();
	Vector getHVec() const 
	{
		return m_H;
	}
	Vector getVVec() const
	{
		return m_V;
	}
private:
	Vector m_Up, m_Center;
	Vector m_H, m_V;
	Vector m_Eye;
	void Init()
	{
    	Vector cameraDirection = (m_Center - m_Eye).normalized();
    	Vector hVec = cameraDirection.cross(m_Up).normalized();
    	Vector vVec = cameraDirection.cross(hVec).normalized();
    	m_H = hVec * m_Up.length();
    	m_V = vVec * m_Up.length();
	}
};
#endif /* end of include guard: CAMERA_H_SDFH987D */