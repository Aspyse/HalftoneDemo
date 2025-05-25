
BufferType& RenderPass::Data()
{

}

bool RenderPass::Initialize(ID3D11Device* device, const wchar_t* pixelFilename)
{
	wchar_t vsFilename[128];
	wchar_t psFilename[128];

	int error = wcscpy_s(psFilename, 128, pixelFilename);
	if (error != 0)
		return false;

	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;

	error = wcscpy_s(vsFilename, 128, L"Shaders/base.vs");
	if (error != 0)
		return false;

	// Compile vertex shader code
	HRESULT result = D3DCompileFromFile(vsFilename, nullptr, nullptr, "BaseVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage)
			OutputShaderErrorMessage(errorMessage, vsFilename);
		return false;
	}

	// Compile pixel shader code
	result = D3DCompileFromFile(psFilename, nullptr, nullptr, "PostprocessShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage)
			OutputShaderErrorMessage(errorMessage, psFilename);
		return false;
	}

	// Create vertex shader from buffer
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), nullptr, &m_vertexShader);
	if (FAILED(result))
		return false;

	// Create pixel shader from buffer
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), nullptr, &m_pixelShader);
	if (FAILED(result))
		return false;

	// Release vertex and pixel shader buffers
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	return true;
}

bool RenderPass::Render(ID3D11DeviceContext* deviceContext)
{
	SetShaderParameters(deviceContext);
	RenderFrame(deviceContext);

	return true;
}

void RenderPass::AssignRenderTarget(RenderTarget* target, ID3D11DepthStencilView* pDepthStencilView)
{
	m_renderTarget = target;
	m_dsv = pDepthStencilView;
}

template<typename BufferType>
bool RenderPass::InitializeConstantBuffer()
{
	m_bufferSize = sizeof(BufferType);

	// Create dynamic matrix constant buffer description
	D3D11_BUFFER_DESC hbd;
	ZeroMemory(&hbd, sizeof(hbd));
	hbd.Usage = D3D11_USAGE_DYNAMIC;
	hbd.ByteWidth = sizeof(BufferType);
	hbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hbd.MiscFlags = 0;
	hbd.StructureByteStride = 0;

	HRESULT result = device->CreateBuffer(&hbd, nullptr, &m_constantBuffer);
	if (FAILED(result))
		return false;

	return true;
}

bool RenderPass::RenderFrame(ID3D11DeviceContext* deviceContext)
{
	deviceContext->IASetInputLayout(nullptr);

	deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	deviceContext->PSSetShader(m_pixelShader, nullptr, 0);

	deviceContext->Draw(3, 0);
}

bool RenderPass::SetShaderParameters(ID3D11DeviceContext* deviceContext)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result = deviceContext->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	memcpy(mappedResource.pData, &m_bufferData, m_bufferSize);

	deviceContext->Unmap(m_constantBuffer, 0);

	UINT bufferNumber = 0;

	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_constantBuffer);

	return true;
}



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