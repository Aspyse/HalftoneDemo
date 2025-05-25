#pragma once

#include <windows.h>
#include <directxmath.h>

using DirectX::XMFLOAT3;
using DirectX::XMMATRIX;

class CameraClass
{
public:
	CameraClass();
	CameraClass(const CameraClass&);
	~CameraClass();

	bool Initialize(float, float, float, float);

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);
	void SetOrbitPosition(float, float, float);

	void Orbit(float, float);
	void Pan(float, float);
	void Zoom(float);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetRotation();

	void Frame(POINT, bool, bool, int);
	XMMATRIX GetViewMatrix() const;
	XMMATRIX GetProjectionMatrix() const;

private:
	const float ORBIT_SENSITIVITY = 0.2f;
	const float PAN_SENSITIVITY = 0.0005f;
	const float ZOOM_SENSITIVITY = 0.0005f;

	XMFLOAT3 m_position = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 m_rotation = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 m_orbitPos = { 0.0f, 0.0f, 0.0f };
	
	XMMATRIX m_viewMatrix = {};
	XMMATRIX m_projectionMatrix = {};

	POINT m_lastMousePos = { 0, 0 };
	float m_yaw = 0.0f, m_pitch = 0.0f;
	float m_distance = 1.0f;
};