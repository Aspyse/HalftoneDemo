#pragma once

#include <d3d11.h>
#include <directxmath.h>

using DirectX::XMFLOAT2;
using DirectX::XMFLOAT3;
using DirectX::XMMATRIX;

class ModelClass
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 uv;
		XMFLOAT3 normal;
	};

public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	bool Initialize(ID3D11Device*, const char*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();
	XMMATRIX GetWorldMatrix();

private:
	bool LoadGLB(VertexType*&, ULONG*&, const char*, float);
	bool LoadPLY(VertexType*&, ULONG*&, const char*);
	bool CalculateNormals(VertexType*, ULONG*);
	bool CalculateUVs(VertexType*, ULONG*);

private:
	ID3D11Buffer *m_vertexBuffer = nullptr, *m_indexBuffer = nullptr;
	UINT m_vertexCount = 0, m_indexCount = 0;
};