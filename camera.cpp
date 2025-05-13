#include "camera.h"

CameraClass::CameraClass() {}

void CameraClass::SetPosition(float x, float y, float z)
{
	positionX = x;
	positionY = y;
	positionZ = z;
}

void CameraClass::SetRotation(float x, float y, float z)
{
	rotationX = x;
	rotationY = y;
	rotationZ = z;
}

XMFLOAT3 CameraClass::GetPosition()
{
	return XMFLOAT3(positionX, positionY, positionZ);
}

XMFLOAT3 CameraClass::GetRotation()
{
	return XMFLOAT3(rotationX, rotationY, rotationZ);
}

void CameraClass::Render()
{
	XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMVECTOR upVector = XMLoadFloat3(&up);

	XMFLOAT3 position = XMFLOAT3(positionX, positionY, positionZ);
	XMVECTOR positionVector = XMLoadFloat3(&position);

	XMFLOAT3 lookAt = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMVECTOR lookAtVector = XMLoadFloat3(&lookAt);

	// Yaw Pitch Roll
	float pitch = rotationX * 0.01745f;
	float yaw = rotationY * 0.01745f;
	float roll = rotationZ * 0.01745f;

	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	// Transform by rotation matrix
	lookAtVector = XMVector3TransformCoord(lookAtVector, rotationMatrix);
	upVector = XMVector3TransformCoord(upVector, rotationMatrix);

	// Translate by position
	lookAtVector = XMVectorAdd(positionVector, lookAtVector);

	// Create view matrix
	viewMatrix = XMMatrixLookAtLH(positionVector, lookAtVector, upVector);
}

void CameraClass::GetViewMatrix(XMMATRIX& output)
{
	output = viewMatrix;
}