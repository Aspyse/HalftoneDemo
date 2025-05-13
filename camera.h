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
	float positionX = 0.0f, positionY = 0.0f, positionZ = 0.0f;
	float rotationX = 0.0f, rotationY = 0.0f, rotationZ = 0.0f;
	XMMATRIX viewMatrix;
};