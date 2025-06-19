#include "material.h"

void MaterialClass::AddIndex(UINT index)
{
	m_indices.push_back(index);
}
void MaterialClass::SetIndices(UINT* indices, UINT indexCount)
{
	m_indices.assign(indices, indices + indexCount);
}
const std::vector<UINT>& MaterialClass::GetIndices() const
{
	return m_indices;
}
UINT MaterialClass::GetIndexCount()
{
	return m_indices.size();
}

void MaterialClass::SetTexture(UINT index, ID3D11ShaderResourceView* texture)
{
	m_textures[index] = texture;
}
ID3D11ShaderResourceView* MaterialClass::GetTexture(UINT index) const
{
	return m_textures[index];
}

void MaterialClass::SetUseTexture(UINT index, bool use)
{
	m_useTextures[index] = use;
}
bool MaterialClass::GetUseTexture(UINT index) const
{
	return m_useTextures[index];
}

void MaterialClass::SetOpaque(bool isOpaque)
{
	m_isOpaque = isOpaque;
}
bool MaterialClass::IsOpaque() const
{
	return m_isOpaque;
}