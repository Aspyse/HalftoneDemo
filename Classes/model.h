#pragma once

#include "material.h"
#include <fastgltf/core.hpp>
#include <d3d11.h>
#include <directxmath.h>
#include <vector>
#include <memory>

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
		XMFLOAT3 tangent;
		XMFLOAT3 binormal;
	};

public:
	ModelClass();
	ModelClass(const ModelClass&);
	~ModelClass();

	bool Initialize(ID3D11Device*, ID3D11DeviceContext*, const char*);
	void Shutdown();
	void SetVertices(ID3D11DeviceContext*);
	void Render(ID3D11Device*, ID3D11DeviceContext*, UINT);

	int GetIndexCount(UINT);
	bool IsOpaque(UINT);
	XMMATRIX GetWorldMatrix();

	UINT GetMaterialCount() const;

private:
	bool LoadGLB(ID3D11Device*, ID3D11DeviceContext*, VertexType*&, const char*);
	bool LoadPLY(VertexType*&, const char*);
	bool CalculateNormals(VertexType*);
	bool DummyUVs(VertexType*);
	bool CalculateModelVectors(VertexType*);

	ID3D11ShaderResourceView* LoadTextureFromIndex(ID3D11Device*, ID3D11DeviceContext*, fastgltf::Asset*, UINT);

private:
	ID3D11Buffer* m_vertexBuffer = nullptr;
	UINT m_vertexCount = 0;
	std::vector<std::unique_ptr<MaterialClass>> m_materials;

	std::vector<ID3D11Buffer*> m_indexBuffers;
};