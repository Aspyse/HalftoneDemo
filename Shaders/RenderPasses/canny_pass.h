#pragma once

#include "render_pass.h"
#include "render_target.h"
#include <DirectXMath.h>

using DirectX::XMFLOAT2;

class CannyPass : public RenderPass
{
private:
	struct GaussianBufferType
	{
		int isDepth;
		float proj43;
		float proj33;
		int kernelRadius;
		
		XMFLOAT2 texelSize;
		float pad[2];

		float weights[16]; // up to max radius

		GaussianBufferType() :
			isDepth(0),
			kernelRadius(5),
			weights(0)
		{ }
	};

	struct SobelBufferType
	{
		XMFLOAT2 offset;
		int isMono;
		float pad0;
	};

	struct NMSBufferType
	{
		XMFLOAT2 offset;
		float pad[2];
	};

	struct DilateBufferType
	{
		XMFLOAT3 inkColor;
		float threshold;
		
		XMFLOAT2 offset;
		float pad[2];
		
		DilateBufferType() :
			inkColor(0.5f, 0.5f, 0.5f),
			threshold(0.6)
		{ }
	};

public:
	CannyPass()
	{
		m_inputs.resize(1);
	}

	bool Initialize(ID3D11Device*, UINT, UINT) override;
	void Update(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, XMVECTOR, XMFLOAT3, UINT, UINT) override;
	std::vector<ParameterControl> GetParameters() override;

private:
	const wchar_t* filename() const override
	{
		return L"";
	}
	const std::vector<std::string> outputs() const override
	{
		return {
			"canny_out"
		};
	}

	std::unique_ptr<RenderTarget> m_pingTarget, m_pongTarget;

	bool Render(ID3D11Device*, ID3D11DeviceContext*, float*) override;
	bool InitializeConstantBuffer(ID3D11Device*) override;

	ID3D11PixelShader *m_pixelShader2 = nullptr, *m_pixelShader3 = nullptr;
	ID3D11PixelShader *m_gaussianShader1 = nullptr, *m_gaussianShader2 = nullptr;

	GaussianBufferType m_gaussianBuffer;
	SobelBufferType m_sobelBuffer;
	NMSBufferType m_nmsBuffer;
	DilateBufferType m_dilateBuffer;
};