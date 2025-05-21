#define NOMINMAX

#include "camera.h"
#include <DirectXMath.h>
using namespace DirectX;

CameraClass::CameraClass() {}
CameraClass::CameraClass(const CameraClass&) {}
CameraClass::~CameraClass() {}

void CameraClass::SetPosition(float x, float y, float z)
{
	m_position.x = x;
	m_position.y = y;
	m_position.z = z;
}

void CameraClass::SetRotation(float x, float y, float z)
{
	m_rotation.x = x;
	m_rotation.y = y;
	m_rotation.z = z;
}

void CameraClass::SetOrbitPosition(float x, float y, float z)
{
	m_orbitPos.x = x;
	m_orbitPos.y = y;
	m_orbitPos.z = z;
}

void CameraClass::Orbit(float x, float y)
{
	m_yaw += x;
	m_pitch += y;

	// Clamp pitch to avoid flipping
	if (m_pitch > 89.0f)
		m_pitch = 89.0f;
	else if (m_pitch < -89.0f)
		m_pitch = -89.02f;

	// Convert degrees to radians
	float yawRad = XMConvertToRadians(m_yaw);
	float pitchRad = XMConvertToRadians(m_pitch);

	// Calculate direction vector (spherical coordinates)
	XMVECTOR dir;
	dir.m128_f32[0] = cosf(pitchRad) * sinf(yawRad); // x
	dir.m128_f32[1] = sinf(pitchRad);                // y
	dir.m128_f32[2] = cosf(pitchRad) * cosf(yawRad); // z
	dir.m128_f32[3] = 0.0f;

	// Compute position = orbitCenter - direction * distance
	XMVECTOR center = XMLoadFloat3(&m_orbitPos);
	XMVECTOR camPos = XMVectorSubtract(center, XMVectorScale(dir, m_distance));

	// Output position
	XMStoreFloat3(&m_position, camPos);

	// Calculate rotation (Euler angles to look at center)
	XMVECTOR forward = XMVector3Normalize(XMVectorSubtract(center, camPos));
	float yaw = atan2f(forward.m128_f32[0], forward.m128_f32[2]);
	float pitch = asinf(-forward.m128_f32[1]);

	// Output Euler rotation in degrees
	m_rotation = {
		XMConvertToDegrees(pitch),
		XMConvertToDegrees(yaw),
		0.0f // Roll is typically 0 in orbit cameras
	};
}

void CameraClass::Pan(float x, float y)
{
	// Convert degrees to radians
	float yawRad = XMConvertToRadians(m_yaw);
	float pitchRad = XMConvertToRadians(m_pitch);
	
	XMVECTOR forward = XMVectorSet(
		cosf(pitchRad) * sinf(yawRad),
		sinf(pitchRad),
		cosf(pitchRad) * cosf(yawRad),
		0.0f
	);
	
	// Calculate right and up vectors from yaw
	XMVECTOR upWorld = XMVectorSet(0, 1, 0, 0);
	XMVECTOR right = XMVector3Normalize(XMVector3Cross(upWorld, forward));
	XMVECTOR up = XMVector3Normalize(XMVector3Cross(forward, right));

	// Apply pan in camera's right/up plane
	XMVECTOR center = XMLoadFloat3(&m_orbitPos);
	center = XMVectorAdd(center, XMVectorScale(right, -x));
	center = XMVectorAdd(center, XMVectorScale(up, y));
	XMStoreFloat3(&m_orbitPos, center);

	// Recalculate camera position after panning
	XMVECTOR newPosition = XMVectorSubtract(center, XMVectorScale(forward, m_distance));
	XMStoreFloat3(&m_position, newPosition);
}

void CameraClass::Zoom(float delta)
{
	m_distance += delta;
	if (m_distance < 0.01f)
		m_distance = 0.01f;

	// Convert degrees to radians
	float yawRad = XMConvertToRadians(m_yaw);
	float pitchRad = XMConvertToRadians(m_pitch);
	
	XMVECTOR forward = XMVectorSet(
		cosf(pitchRad) * sinf(yawRad),
		sinf(pitchRad),
		cosf(pitchRad) * cosf(yawRad),
		0.0f
	);

	XMVECTOR center = XMLoadFloat3(&m_orbitPos);

	XMVECTOR newPosition = XMVectorSubtract(center, XMVectorScale(forward, m_distance));
	XMStoreFloat3(&m_position, newPosition);
}

XMFLOAT3 CameraClass::GetPosition()
{
	return XMFLOAT3(m_position.x, m_position.y, m_position.z);
}

XMFLOAT3 CameraClass::GetRotation()
{
	return XMFLOAT3(m_rotation.x, m_rotation.y, m_rotation.z);
}

void CameraClass::Frame(InputSystem* inputHandle)
{
	POINT newPos = inputHandle->GetMousePos();
	POINT delta = {
		newPos.x - m_lastMousePos.x,
		newPos.y - m_lastMousePos.y
	};
	m_lastMousePos = newPos;
	if (inputHandle->IsMiddleMouseDown())
	{
		if (inputHandle->IsKeyDown(VK_SHIFT))
			Pan(delta.x * PAN_SENSITIVITY, delta.y * PAN_SENSITIVITY);
		else
			Orbit(delta.x * ORBIT_SENSITIVITY, -delta.y * ORBIT_SENSITIVITY);
	}
	int scrollDelta = inputHandle->GetScrollDelta();
	if (scrollDelta != 0)
		Zoom(-scrollDelta * ZOOM_SENSITIVITY);

	XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMVECTOR upVector = XMLoadFloat3(&up);

	XMVECTOR positionVector = XMLoadFloat3(&m_position);

	XMFLOAT3 lookAt = XMFLOAT3(0.0f, 0.0f, 1.0f);
	XMVECTOR lookAtVector = XMLoadFloat3(&lookAt);

	// Yaw Pitch Roll
	float pitch = m_rotation.x * 0.01745f;
	float yaw = m_rotation.y * 0.01745f;
	float roll = m_rotation.z * 0.01745f;

	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	// Transform by rotation matrix
	lookAtVector = XMVector3TransformCoord(lookAtVector, rotationMatrix);
	upVector = XMVector3TransformCoord(upVector, rotationMatrix);

	// Translate by position
	lookAtVector = XMVectorAdd(positionVector, lookAtVector);

	// Create view matrix
	m_viewMatrix = XMMatrixLookAtLH(positionVector, lookAtVector, upVector);
}

void CameraClass::GetViewMatrix(XMMATRIX& output)
{
	output = m_viewMatrix;
}