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
		XMFLOAT4 color;
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
	ID3D11Buffer *vertexBuffer = nullptr, *indexBuffer = nullptr;
	int vertexCount = 0, indexCount = 0;
};