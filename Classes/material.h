#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

class MaterialClass
{
public:
	struct TexTransform
	{
		DirectX::XMFLOAT2 offset;
		DirectX::XMFLOAT2 scale;
		float rotation;
	};

	MaterialClass() {};
	
	void AddIndex(UINT);
	void SetIndices(UINT*, UINT);
	const std::vector<UINT>& GetIndices() const;
	UINT GetIndexCount();

	void SetTexture(UINT, ID3D11ShaderResourceView*);
	ID3D11ShaderResourceView* GetTexture(UINT) const;

	void SetUseTexture(UINT, bool);
	bool GetUseTexture(UINT) const;

	void SetOpaque(bool);
	bool IsOpaque() const;

private:
	// [0] = albedo, [1] = normal, [2] = roughness
	ID3D11ShaderResourceView* m_textures[3] = { nullptr, nullptr, nullptr };
	bool m_useTextures[3] = { true, true, true };

	TexTransform m_albedoTransform;
	std::vector<UINT> m_indices;

	bool m_isOpaque = true;
};