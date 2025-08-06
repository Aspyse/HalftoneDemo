#pragma once

#include "render_target.h"
#include <fstream>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <functional> // TODO: check if expensive overhead
#include <variant>
#include <vector>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;
using std::ofstream;
using DirectX::XMMATRIX;
using DirectX::XMVECTOR;
using DirectX::XMFLOAT3;

class RenderPass
{
	friend class Effect;
	
	using ParamRef = std::variant<
		std::reference_wrapper<float>,
		std::reference_wrapper<XMFLOAT3>,
		std::reference_wrapper<int>,
		std::reference_wrapper<bool>,
		std::reference_wrapper<std::string>
	>;

public:
	enum WidgetType
	{
		FLOAT,
		FLOAT3,
		COLOR,
		INT,
		ANGLE,
		CHECKBOX,
		RENDER_TARGET
	};

	struct ParameterControl
	{
		std::string m_name;
		WidgetType m_type;
		ParamRef m_field;
	};

	virtual bool Initialize(ID3D11Device*, UINT, UINT);
	virtual bool Render(ID3D11Device*, ID3D11DeviceContext*, float*);
	virtual std::vector<ParameterControl> GetParameters() = 0;
	virtual void Update(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, XMVECTOR, XMFLOAT3, UINT, UINT) = 0;

	std::vector<std::string> GetInputs() const;
	std::vector<std::string> GetOutputs() const;

	static void OutputShaderErrorMessage(ID3D10Blob* errorMessage, WCHAR* shaderFilename)
	{
		char* compileErrors;
		unsigned long long bufferSize, i;
		ofstream fout;

		compileErrors = (char*)(errorMessage->GetBufferPointer());

		bufferSize = errorMessage->GetBufferSize();
		fout.open("shader-error.txt");
		for (i = 0; i < bufferSize; i++)
		{
			fout << compileErrors[i];
		}
		fout.close();

		errorMessage->Release();
		errorMessage = 0;
	}


protected:
	template <typename T>
	void AddCB(ID3D11Device* device)
	{
		ComPtr<ID3D11Buffer> tempBuffer;

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(T);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd.MiscFlags = 0;
		bd.StructureByteStride = 0;

		device->CreateBuffer(&bd, nullptr, tempBuffer.GetAddressOf());

		m_constantBuffers.push_back(tempBuffer);
	}

	virtual void Begin(ID3D11Device*, ID3D11DeviceContext*) { } // Optional pre-setup, e.g. bind extra samplers
	virtual bool InitializeConstantBuffer(ID3D11Device* device) = 0;
	virtual const wchar_t* filename() const = 0;
	virtual const std::vector<std::string> outputs() const = 0;


protected:
	std::vector<ComPtr<ID3D11Buffer>> m_constantBuffers;
	std::vector<std::string> m_inputs;

	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_pixelShader = nullptr;
};
