#include "model.h"
#include <miniply.h>
#include <fastgltf/tools.hpp>
#include <WICTextureLoader.h>

ModelClass::ModelClass() {}
ModelClass::ModelClass(const ModelClass&) {}
ModelClass::~ModelClass() {}

using namespace DirectX;

bool ModelClass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* filename)
{
	VertexType* vertices = nullptr;
	
	const char* extension = strrchr(filename, '.');
	if (strcmp(extension, ".ply") == 0)
	{
		m_materials.push_back(std::make_unique<MaterialClass>());
		LoadPLY(vertices, filename);
		CalculateNormals(vertices);
		DummyUVs(vertices);
	}
	else if (strcmp(extension, ".glb") == 0)
		LoadGLB(device, deviceContext, vertices, filename);
	else
		return false;

	CalculateModelVectors(vertices);

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

	m_indexBuffers.resize(m_materials.size());
	for (UINT i = 0; i < m_materials.size(); ++i)
	{
		// Create description of static index buffer
		D3D11_BUFFER_DESC ibd;
		ZeroMemory(&ibd, sizeof(ibd));
		ibd.Usage = D3D11_USAGE_DEFAULT;
		ibd.ByteWidth = sizeof(ULONG) * m_materials[i]->GetIndices().size();
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		ibd.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA indexData;
		ZeroMemory(&indexData, sizeof(indexData));
		indexData.pSysMem = m_materials[i]->GetIndices().data();
		indexData.SysMemPitch = 0;
		indexData.SysMemSlicePitch = 0;

		HRESULT result = device->CreateBuffer(&ibd, &indexData, &m_indexBuffers[i]);
	}

	delete[] vertices;
	vertices = nullptr;

	return true;
}

int ModelClass::GetIndexCount(UINT materialIndex)
{
	return m_materials[materialIndex]->GetIndexCount();
}

bool ModelClass::IsOpaque(UINT materialIndex)
{
	return m_materials[materialIndex]->IsOpaque();
}

XMMATRIX ModelClass::GetWorldMatrix()
{
	// TODO: actually handle
	return XMMatrixIdentity();
}

void ModelClass::Shutdown()
{
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}
}

void ModelClass::SetVertices(ID3D11DeviceContext* deviceContext)
{
	UINT stride = sizeof(VertexType);
	UINT offset = 0;
	
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// Set type of primitive to be rendered
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ModelClass::Render(ID3D11Device* device, ID3D11DeviceContext* deviceContext, UINT materialIndex)
{
	// Set vertex and index buffers to active
	deviceContext->IASetIndexBuffer(m_indexBuffers[materialIndex], DXGI_FORMAT_R32_UINT, 0);

	ID3D11ShaderResourceView* srv = m_materials[materialIndex]->GetTexture(0);
	deviceContext->PSSetShaderResources(0, 1, &srv);
}

bool ModelClass::LoadGLB(ID3D11Device* device, ID3D11DeviceContext* deviceContext, VertexType*& outVertices, const char* filename)
{
	std::vector<VertexType> vertices;
	
	auto ext =
		fastgltf::Extensions::KHR_mesh_quantization |
		fastgltf::Extensions::KHR_texture_transform;
	
	fastgltf::Parser parser{ ext };
	std::filesystem::path filePath(filename);

	auto data = fastgltf::GltfDataBuffer::FromPath(filePath);
	if (data.error() != fastgltf::Error::None)
		return false;

	auto asset = parser.loadGltfBinary(data.get(), filePath.parent_path(), fastgltf::Options::None);
	if (asset.error() != fastgltf::Error::None)
		return false;

	// Set up materials
	for (UINT i = 0; i < asset->materials.size(); ++i)
	{
		m_materials.push_back(std::make_unique<MaterialClass>());
		
		m_materials[i]->SetOpaque(asset->materials[i].alphaMode == fastgltf::AlphaMode::Opaque);
		if (asset->materials[i].pbrData.baseColorTexture.has_value())
		{
			m_materials[i]->SetTexture(0, LoadTextureFromIndex(device, deviceContext, &asset.get(), i));
		}


	}

	UINT defaultScene = 0;
	if (asset->defaultScene.has_value())
		defaultScene = asset->defaultScene.value();
	
	const auto& scene = asset->scenes[defaultScene];

	for (auto nodeIndex : scene.nodeIndices) {
		const auto& node = asset->nodes[nodeIndex];
		if (!node.meshIndex.has_value())
			continue; // TODO: check if this is correct handling
		const auto& mesh = asset->meshes[node.meshIndex.value()];
		const auto& trs = std::get<fastgltf::TRS>(node.transform); // TODO: check, may not work for all models?

		for (const auto& primitive : mesh.primitives) {
			UINT baseVertex = vertices.size();

			// === Indices ===
			std::vector<uint32_t> localIndices;
			if (primitive.indicesAccessor) {
				auto& accessor = asset->accessors[*primitive.indicesAccessor];
				localIndices.resize(accessor.count);
				fastgltf::iterateAccessorWithIndex<uint32_t>(asset.get(), accessor,
					[&](uint32_t val, UINT i) {
						localIndices[i] = static_cast<ULONG>(val);
					});
			}

			// === Attributes ===
			std::vector<fastgltf::math::fvec3> positions;
			std::vector<fastgltf::math::fvec3> normals;
			std::vector<fastgltf::math::fvec2> uvs;

			// POSITION
			if (auto* it = primitive.findAttribute("POSITION"); it != primitive.attributes.end()) {
				auto& accessor = asset->accessors[it->accessorIndex];
				positions.resize(accessor.count);
				fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset.get(), accessor,
					[&](fastgltf::math::fvec3 val, UINT i) {
						positions[i] = val;
					});
			}

			// NORMAL
			if (auto it = primitive.findAttribute("NORMAL"); it != primitive.attributes.end()) {
				auto& accessor = asset->accessors[it->accessorIndex];
				normals.resize(accessor.count);
				fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset.get(), accessor,
					[&](fastgltf::math::fvec3 val, UINT i) {
						normals[i] = val;
					});
			}

			// TEXCOORD_0
			if (auto it = primitive.findAttribute("TEXCOORD_0"); it != primitive.attributes.end()) {
				auto& accessor = asset->accessors[it->accessorIndex];
				uvs.resize(accessor.count);
				fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(asset.get(), accessor,
					[&](fastgltf::math::fvec2 val, UINT i) {
						uvs[i] = val;
					});
			}

			// Transform UVs
			if (primitive.materialIndex.has_value())
			{
				const auto& mat = asset->materials[primitive.materialIndex.value()];
				if (mat.pbrData.baseColorTexture.has_value())
				{
					const auto& baseColorTexture = mat.pbrData.baseColorTexture.value();
					auto transform = baseColorTexture.transform.get();

					for (auto& uv : uvs)
					{
						uv *= transform->uvScale;

						float cosR = std::cos(transform->rotation);
						float sinR = std::sin(transform->rotation);
						float x = uv[0];
						float y = uv[1];
						uv[0] = cosR * x - sinR * y;
						uv[1] = sinR * x + cosR * y;

						uv += transform->uvOffset;
					}
				}
				else continue;
			}
			else continue;

			// Assemble vertices 
			UINT count = positions.size();  // assume all attributes match count
			vertices.reserve(vertices.size() + count);
			for (UINT i = 0; i < count; ++i) {
				//positions[i] *= scaleFac; // TODO: clean up temp workaround

				/* HANDLE TRANSFORMATION */
				//TODO: handle rotation
				positions[i] *= trs.scale;
				positions[i] += trs.translation;

				VertexType v = {
					i < positions.size() ? DirectX::XMFLOAT3(positions[i][0], positions[i][1], positions[i][2]) : DirectX::XMFLOAT3(0,0,0),
					i < uvs.size() ? DirectX::XMFLOAT2(uvs[i][0], uvs[i][1]) : DirectX::XMFLOAT2(0,0), // TODO: figure out why
					i < normals.size() ? DirectX::XMFLOAT3(normals[i][0], normals[i][1], normals[i][2]) : DirectX::XMFLOAT3(0,0,0)
				};
				vertices.push_back(v);
			}

			for (uint32_t localIdx : localIndices) {
				UINT index = 0;
				if (primitive.materialIndex.has_value())
					index = primitive.materialIndex.value();

				m_materials[index]->AddIndex(static_cast<UINT>(localIdx + baseVertex));
			}
		}
	}

	// TODO: send vertices to outVertices
	m_vertexCount = vertices.size();
	outVertices = new VertexType[m_vertexCount];

	//std::memcpy(outVertices, vertices.data(), m_vertexCount * sizeof(VertexType));
	std::copy(
		vertices.begin(),
		vertices.end(),
		outVertices
	);
	
	return true;
}

ID3D11ShaderResourceView* ModelClass::LoadTextureFromIndex(ID3D11Device* device, ID3D11DeviceContext* context, fastgltf::Asset* asset, UINT materialIndex)
{
	ID3D11ShaderResourceView* outSRV;

	auto textureIndex = asset->materials[materialIndex].pbrData.baseColorTexture.value().textureIndex;
	if (!asset->textures[textureIndex].imageIndex.has_value())
		return false;

	const auto& imageIndex = asset->textures[textureIndex].imageIndex.value();
	const auto& image = asset->images[imageIndex];

	if (std::holds_alternative<fastgltf::sources::BufferView>(image.data)) {
		const auto& bufferViewImage = std::get<fastgltf::sources::BufferView>(image.data);

		const auto& bufferView = asset->bufferViews[bufferViewImage.bufferViewIndex];
		const auto& buffer = asset->buffers[bufferView.bufferIndex];
		
		const auto& bufferArray = std::get<fastgltf::sources::Array>(buffer.data);
		const uint8_t* imageData = reinterpret_cast<const uint8_t*>(bufferArray.bytes.data()) + bufferView.byteOffset;
		size_t imageSize = bufferView.byteLength;

		HRESULT hr = DirectX::CreateWICTextureFromMemory(
			device,
			context,
			imageData,
			imageSize,
			nullptr,    // [out] ID3D11Resource** (optional)
			&outSRV     // [out] SRV
		);
		if (FAILED(hr))
		{
			OutputDebugStringA("Failed to CreateWICTextureFromMemory\n");
			return nullptr;
		}

		return outSRV;
	}


	return nullptr;
}

bool ModelClass::LoadPLY(VertexType*& outVertices, const char* filename)
{
	miniply::PLYReader reader(filename);

	uint32_t faceIdxs[3];
	miniply::PLYElement* faceElem = reader.get_element(reader.find_element(miniply::kPLYFaceElement));
	if (faceElem == nullptr)
		return false;

	faceElem->convert_list_to_fixed_size(faceElem->find_property("vertex_indices"), 3, faceIdxs);

	UINT indexes[3];
	bool gotVerts = false, gotFaces = false;
	float* pos = nullptr;

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
			UINT indexCount = reader.num_rows() * 3;
			UINT* outIndices = new UINT[indexCount];
			reader.extract_properties(faceIdxs, 3, miniply::PLYPropertyType::Int, outIndices);

			m_materials[0]->SetIndices(outIndices, indexCount);
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

	return true;
}

bool ModelClass::CalculateNormals(VertexType* vertices)
{	
	auto indices = m_materials[0]->GetIndices();
	
	// Initialize all normals to zero
	for (ULONG i = 0; i < m_vertexCount; ++i)
	{
		vertices[i].normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}

	// Loop over each triangle
	for (UINT i = 0; i < m_materials[0]->GetIndexCount(); i += 3) {
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

bool ModelClass::DummyUVs(VertexType* vertices)
{
	for (UINT i = 0; i < m_vertexCount; ++i)
		vertices[i].uv = XMFLOAT2(0.0f, 0.0f);

	return true;
}

bool ModelClass::CalculateModelVectors(VertexType* vertices)
{
	return true;
}

UINT ModelClass::GetMaterialCount() const
{
	return m_materials.size();
}