#pragma once

#include <d3d11.h>
#include <directxmath.h>
using namespace DirectX;

class ModelClass
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT3 normal;
	};

public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	bool Initialize(ID3D11Device*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();

private:
	bool LoadPLY(VertexType*&, ULONG*&, const char*);
	bool CalculateNormals(VertexType*&, ULONG*);

private:
	ID3D11Buffer *m_vertexBuffer = nullptr, *m_indexBuffer = nullptr;
	UINT m_vertexCount = 0, m_indexCount = 0;
};