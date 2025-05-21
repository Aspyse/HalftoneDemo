#include "model.h"
#include <miniply.h>

ModelClass::ModelClass() {}
ModelClass::ModelClass(const ModelClass&) {}
ModelClass::~ModelClass() {}

bool ModelClass::LoadPLY(VertexType*& outVertices, ULONG*& outIndices, const char* filename)
{
	miniply::PLYReader reader(filename);
	
	uint32_t faceIdxs[3];
	miniply::PLYElement* faceElem = reader.get_element(reader.find_element(miniply::kPLYFaceElement));
	if (faceElem == nullptr)
		return false;
	
	faceElem->convert_list_to_fixed_size(faceElem->find_property("vertex_indices"), 3, faceIdxs);

	UINT indexes[3];
	bool gotVerts = false, gotFaces = false;
	float *pos = nullptr;

	while (reader.has_element() && (!gotVerts || !gotFaces))
	{
		if (reader.element_is(miniply::kPLYVertexElement) && reader.load_element() && reader.find_pos(indexes))
		{
			m_vertexCount = reader.num_rows();
			pos = new float[m_vertexCount * 3];
			outVertices = new VertexType[m_vertexCount];
			reader.extract_properties(indexes, 3, miniply::PLYPropertyType::Float, pos);

			for (UINT i = 0; i < m_vertexCount; ++i)
				outVertices[i].position = XMFLOAT3(pos[i * 3 + 0], pos[i * 3 + 1], pos[i * 3 + 2]);

			gotVerts = true;
		}
		else if (!gotFaces && reader.element_is(miniply::kPLYFaceElement) && reader.load_element())
		{
			m_indexCount = reader.num_rows() * 3;
			outIndices = new ULONG[m_indexCount];
			reader.extract_properties(faceIdxs, 3, miniply::PLYPropertyType::Int, outIndices);
			gotFaces = true;
		}
		if (gotVerts && gotFaces)
		{
			break;
		}
		reader.next_element();
	}

	delete[] pos;
	pos = nullptr;

	if (!gotVerts || !gotFaces)
		return false;

	CalculateNormals(outVertices, outIndices);
	CalculateUVs(outVertices, outIndices);

	return true;
}

bool ModelClass::CalculateNormals(VertexType* vertices, ULONG* indices)
{
	// Initialize all normals to zero
	for (ULONG i = 0; i < m_vertexCount; ++i)
	{
		vertices[i].normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}

	// Calculate normals for each face and add them to associated vertices
	/*for (ULONG i = 0; i < m_indexCount; i += 3)
	{
		ULONG i0 = indices[i];
		ULONG i1 = indices[i+1];
		ULONG i2 = indices[i+2];
		// THANKS POITA FROM DEVMASTER 2010
		XMVECTOR v[3] = {
			XMLoadFloat3(&vertices[i0].position),
			XMLoadFloat3(&vertices[i1].position),
			XMLoadFloat3(&vertices[i2].position)
		};

		// Calculate the cross product to get the normal vector
		XMVECTOR normal = XMVector3Cross(v[1]-v[0], v[1]-v[2]);

		// Store the normal temporarily
		XMFLOAT3 faceNormal;
		XMStoreFloat3(&faceNormal, normal);

		for (int j = 0; j < 3; ++j)
		{
			XMVECTOR a = v[(j + 1) % 3] - v[j];
			XMVECTOR b = v[(j + 2) % 3] - v[j];

			float dot = XMVectorGetX(XMVector3Dot(a, b));
			float lenA = XMVectorGetX(XMVector3Length(a));
			float lenB = XMVectorGetX(XMVector3Length(b));
			float weight = acos(dot / (lenA * lenB));

			XMVECTOR current = XMLoadFloat3(&vertices[indices[i + j]].normal);
			current += weight * normal;
			XMStoreFloat3(&vertices[indices[i + j]].normal, current);
		}
	}*/

	// Loop over each triangle
	for (size_t i = 0; i < m_indexCount; i += 3) {
		ULONG i0 = indices[i];
		ULONG i1 = indices[i + 1];
		ULONG i2 = indices[i + 2];

		XMVECTOR v0 = XMLoadFloat3(&vertices[i0].position);
		XMVECTOR v1 = XMLoadFloat3(&vertices[i1].position);
		XMVECTOR v2 = XMLoadFloat3(&vertices[i2].position);

		// Calculate the two edge vectors
		XMVECTOR edge1 = XMVectorSubtract(v1, v0);
		XMVECTOR edge2 = XMVectorSubtract(v2, v0);

		// Compute the face normal via cross product
		XMVECTOR faceNormal = XMVector3Cross(edge1, edge2);
		faceNormal = XMVector3Normalize(faceNormal);

		// Accumulate the face normal into each vertex's normal
		for (ULONG idx : { i0, i1, i2 }) {
			XMVECTOR n = XMLoadFloat3(&vertices[idx].normal);
			n = XMVectorAdd(n, faceNormal);
			XMStoreFloat3(&vertices[idx].normal, n);
		}
	}

	// Normalize the accumulated normals for each vertex
	for (ULONG i = 0; i < m_vertexCount; ++i)
	{
		XMVECTOR normal = XMLoadFloat3(&vertices[i].normal);
		normal = XMVector3Normalize(normal);
		XMStoreFloat3(&vertices[i].normal, normal);
	}

	return true;
}

bool ModelClass::CalculateUVs(VertexType* vertices, ULONG* indices) {
	for (UINT i = 0; i < m_vertexCount; ++i)
		vertices[i].uv = XMFLOAT2(0.0f, 0.0f);

	return true;
}

bool ModelClass::Initialize(ID3D11Device* device, const char* filename)
{
	VertexType* vertices = nullptr;
	ULONG* indices = nullptr;
	
	LoadPLY(vertices, indices, filename);

	// Create description of static vertex buffer
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vertexData;
	ZeroMemory(&vertexData, sizeof(vertexData));
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	HRESULT result = device->CreateBuffer(&vbd, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
		return false;

	// Create description of static index buffer
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(ULONG) * m_indexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexData;
	ZeroMemory(&indexData, sizeof(indexData));
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&ibd, &indexData, &m_indexBuffer);
	if (FAILED(result))
		return false;

	delete[] vertices;
	vertices = nullptr;
	delete[] indices;
	indices = nullptr;

	return true;
}

int ModelClass::GetIndexCount()
{
	return m_indexCount;
}

void ModelClass::Shutdown()
{
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = nullptr;
	}
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}
}

void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
	UINT stride = sizeof(VertexType);
	UINT offset = 0;

	// Set vertex and index buffers to active
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set type of primitive to be rendered
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
