#pragma once

#include "input_system.h"
#include <directxmath.h>
using namespace DirectX;

class CameraClass
{
public:
	CameraClass();
	CameraClass(const CameraClass&);
	~CameraClass();

	void Initialize();

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);
	void SetOrbitPosition(float, float, float);

	void Orbit(float, float);
	void Pan(float, float);
	void Zoom(float);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetRotation();

	void Frame(InputSystem*);
	void GetViewMatrix(XMMATRIX&);

private:
	const float ORBIT_SENSITIVITY = 0.2f;
	const float PAN_SENSITIVITY = 0.0005f;
	const float ZOOM_SENSITIVITY = 0.0005f;

	XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 m_rotation = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 m_orbitPos = { 0.0f, 0.0f, 0.0f };
	
	XMMATRIX m_viewMatrix = {};

	POINT m_lastMousePos = { 0, 0 };
	float m_yaw = 0.0f, m_pitch = 0.0f;
	float m_distance = 1.0f;
};