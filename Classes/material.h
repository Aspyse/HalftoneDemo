#pragma once

#include <d3d11.h>
#include <vector>

class MaterialClass
{
public:
	MaterialClass() {};
	
	void AddIndex(UINT index)
	{
		m_indices.push_back(index);
	}
	void SetIndices(UINT* indices, UINT indexCount)
	{
		m_indices.assign(indices, indices+indexCount);
	}
	const std::vector<UINT>& GetIndices() const
	{
		return m_indices;
	}
	UINT GetIndexCount()
	{
		return m_indices.size();
	}

	void SetTexture(UINT index, ID3D11ShaderResourceView* texture)
	{
		m_textures[index] = texture;
	}
	ID3D11ShaderResourceView* GetTexture(UINT index) const
	{
		return m_textures[index];
	}

	void SetOpaque(bool isOpaque)
	{
		m_isOpaque = isOpaque;
	}
	bool IsOpaque()
	{
		return m_isOpaque;
	}

private:
	// [0] = albedo, [1] = normal, [2] = roughness
	ID3D11ShaderResourceView* m_textures[3] = { nullptr, nullptr, nullptr };

	std::vector<UINT> m_indices;

	bool m_isOpaque = true;
};