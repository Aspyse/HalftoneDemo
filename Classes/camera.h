#pragma once

#include <directxmath.h>
using namespace DirectX;

class CameraClass
{
public:
	CameraClass();
	CameraClass(const CameraClass&);
	~CameraClass();

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetRotation();

	void Render();
	void GetViewMatrix(XMMATRIX&);

private:
	float m_positionX = 0.0f, m_positionY = 0.0f, m_positionZ = 0.0f;
	float m_rotationX = 0.0f, m_rotationY = 0.0f, m_rotationZ = 0.0f;
	XMMATRIX m_viewMatrix = {};
};