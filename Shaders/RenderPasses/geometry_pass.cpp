#include "geometry_pass.h"
#include <fstream>

GeometryPass::GeometryPass() {}
GeometryPass::GeometryPass(const GeometryPass&) {}
GeometryPass::~GeometryPass() {}

ComPtr<ID3D11ShaderResourceView> GeometryPass::GetGBuffer(int index)
{
    switch (index)
    {
    case 0:
        return m_albedoSRV;
        break;
    case 1:
        return m_normalSRV;
        break;
    case 2:
        return m_depthSRV;
        break;
    }
}

ComPtr<ID3D11ShaderResourceView> GeometryPass::GetShadowMap()
{
    return m_shadowSRV;
}

XMMATRIX GeometryPass::GetLightViewProj() const
{
    return m_lightViewProj;
}

bool GeometryPass::Initialize(ID3D11Device* device, UINT width, UINT height)
{
	if (!CompileShader(device))
		return false;

    // Texture sampler
    if (!InitializeSampler(device))
        return false;

    if (!InitializeGBuffer(device, width, height))
        return false;

    if (!InitializeShadow(device))
        return false;

	return true;
}

bool GeometryPass::CompileShader(ID3D11Device* device)
{
    ID3D10Blob* errorMessage = nullptr;
    ID3D10Blob* vertexShaderBuffer = nullptr;
    ID3D10Blob* pixelShaderBuffer = nullptr;

    wchar_t vsFilename[128], psFilename[128];

    int error = wcscpy_s(vsFilename, 128, L"Shaders/geometry.vs");
    if (error != 0)
        return false;

    error = wcscpy_s(psFilename, 128, L"Shaders/geometry.ps");
    if (error != 0)
        return false;
    
    HRESULT result = D3DCompileFromFile(vsFilename, nullptr, nullptr, "GeometryVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
    if (FAILED(result))
    {
        if (errorMessage)
            OutputShaderErrorMessage(errorMessage, vsFilename);
        return false;
    }
    result = D3DCompileFromFile(psFilename, nullptr, nullptr, "GeometryPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
    if (FAILED(result))
    {
        if (errorMessage)
            OutputShaderErrorMessage(errorMessage, psFilename);
        return false;
    }

    result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), nullptr, &m_vertexShader);
    if (FAILED(result))
        return false;

    // Create pixel shader from buffer
    result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), nullptr, &m_pixelShader);
    if (FAILED(result))
        return false;

    D3D11_INPUT_ELEMENT_DESC pl[4];
    pl[0].SemanticName = "POSITION";
    pl[0].SemanticIndex = 0;
    pl[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    pl[0].InputSlot = 0;
    pl[0].AlignedByteOffset = 0;
    pl[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    pl[0].InstanceDataStepRate = 0;

    pl[1].SemanticName = "TEXCOORD";
    pl[1].SemanticIndex = 0;
    pl[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    pl[1].InputSlot = 0;
    pl[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    pl[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    pl[1].InstanceDataStepRate = 0;

    pl[2].SemanticName = "NORMAL";
    pl[2].SemanticIndex = 0;
    pl[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    pl[2].InputSlot = 0;
    pl[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    pl[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    pl[2].InstanceDataStepRate = 0;

    pl[3].SemanticName = "TANGENT";
    pl[3].SemanticIndex = 0;
    pl[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    pl[3].InputSlot = 0;
    pl[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    pl[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    pl[3].InstanceDataStepRate = 0;


    UINT numElements = sizeof(pl) / sizeof(pl[0]);

    result = device->CreateInputLayout(pl, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
    if (FAILED(result))
        return false;

    // Release vertex and pixel shader buffers
    vertexShaderBuffer->Release();
    vertexShaderBuffer = 0;

    pixelShaderBuffer->Release();
    pixelShaderBuffer = 0;

    // Create camera buffer description
    D3D11_BUFFER_DESC cbd;
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.ByteWidth = sizeof(CameraBufferType);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbd.MiscFlags = 0;
    cbd.StructureByteStride = 0;

    result = device->CreateBuffer(&cbd, nullptr, &m_cameraBuffer);
    if (FAILED(result))
        return false;

    D3D11_BUFFER_DESC mbd;
    mbd.Usage = D3D11_USAGE_DYNAMIC;
    mbd.ByteWidth = sizeof(MaterialBufferType);
    mbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    mbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    mbd.MiscFlags = 0;
    mbd.StructureByteStride = 0;

    result = device->CreateBuffer(&mbd, nullptr, &m_materialBuffer);
    if (FAILED(result))
        return false;

    return true;
}

bool GeometryPass::InitializeSampler(ID3D11Device* device)
{
    D3D11_SAMPLER_DESC sd;
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.MipLODBias = 0.0f;
    sd.MaxAnisotropy = 1;
    sd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    sd.BorderColor[0] = 0;
    sd.BorderColor[1] = 0;
    sd.BorderColor[2] = 0;
    sd.BorderColor[3] = 0;
    sd.MinLOD = 0;
    sd.MaxLOD = D3D11_FLOAT32_MAX;

    // Create the texture sampler state.
    HRESULT result = device->CreateSamplerState(&sd, &m_sampleStateWrap);
    if (FAILED(result))
        return false;

    return true;
}

bool GeometryPass::InitializeGBuffer(ID3D11Device* device, UINT width, UINT height)
{
    D3D11_TEXTURE2D_DESC td;
    ZeroMemory(&td, sizeof(td));
    td.Width = width;
    td.Height = height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    // Albedo render target
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    ID3D11Texture2D* albedoTex = nullptr;
    HRESULT result = device->CreateTexture2D(&td, nullptr, &albedoTex);
    if (FAILED(result))
        return false;

    // Normal + Roughness render target
    td.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    ID3D11Texture2D* normalTex = nullptr;
    result = device->CreateTexture2D(&td, nullptr, &normalTex);
    if (FAILED(result))
        return false;

    // Albedo RTV
    device->CreateRenderTargetView(albedoTex, nullptr, &m_albedoRTV);

    // Normal+Roughness RTV
    device->CreateRenderTargetView(normalTex, nullptr, &m_normalRTV);

    // Albedo SRV
    device->CreateShaderResourceView(albedoTex, nullptr, &m_albedoSRV);

    // Normal+Roughness SRV
    device->CreateShaderResourceView(normalTex, nullptr, &m_normalSRV);

    // Release textures
    albedoTex->Release();
    albedoTex = nullptr;
    normalTex->Release();
    normalTex = nullptr;

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC depthDesc;
    ZeroMemory(&depthDesc, sizeof(depthDesc));
    depthDesc.Width = width;
    depthDesc.Height = height;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_R24G8_TYPELESS; // Allows both depth-stencil and shader resource views
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    depthDesc.CPUAccessFlags = 0;
    depthDesc.MiscFlags = 0;

    ID3D11Texture2D* depthTexture;
    result = device->CreateTexture2D(&depthDesc, nullptr, &depthTexture);
    if (FAILED(result))
        return false;

    // Create depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    ZeroMemory(&dsvDesc, sizeof(dsvDesc));
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;

    device->CreateDepthStencilView(depthTexture, &dsvDesc, &m_dsv);

    // Create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    device->CreateShaderResourceView(depthTexture, &srvDesc, &m_depthSRV);

    depthTexture->Release();
    depthTexture = nullptr;

    return true;
}

void GeometryPass::Shutdown()
{
    if (m_shadowSRV)
    {
        m_shadowSRV->Release();
        m_shadowSRV = nullptr;
    }
    if (m_shadowDSV)
    {
        m_shadowDSV->Release();
        m_shadowDSV = nullptr;
    }
    if (m_shadowShader)
    {
        m_shadowShader->Release();
        m_shadowShader = nullptr;
    }
    if (m_normalSRV)
    {
        m_normalSRV->Release();
        m_normalSRV = nullptr;
    }
    if (m_normalRTV)
    {
        m_normalRTV->Release();
        m_normalRTV = nullptr;
    }
    if (m_albedoSRV)
    {
        m_albedoSRV->Release();
        m_albedoSRV = nullptr;
    }
    if (m_albedoRTV)
    {
        m_albedoRTV->Release();
        m_albedoRTV = nullptr;
    }
    if (m_materialBuffer)
    {
        m_materialBuffer->Release();
        m_materialBuffer = nullptr;
    }
    if (m_cameraBuffer)
    {
        m_cameraBuffer->Release();
        m_cameraBuffer = nullptr;
    }
    if (m_sampleStateWrap)
    {
        m_sampleStateWrap->Release();
        m_sampleStateWrap = nullptr;
    }
    if (m_layout)
    {
        m_layout->Release();
        m_layout = nullptr;
    }
    if (m_pixelShader)
    {
        m_pixelShader->Release();
        m_pixelShader = nullptr;
    }
    if (m_vertexShader)
    {
        m_vertexShader->Release();
        m_vertexShader = nullptr;
    }
}

std::vector<RenderPass::ParameterControl> GeometryPass::GetParameters()
{
    return {
        //{ "Use Albedo Texture?", RenderPass::WidgetType::CHECKBOX, std::ref(m_materialData.useAlbedoTexture) },
        { "Albedo Color", RenderPass::WidgetType::COLOR, std::ref(m_materialData.albedoColor) },
        //{ "Use Roughness Texture?", RenderPass::WidgetType::CHECKBOX, std::ref(m_materialData.useRoughnessTexture) },
        { "Roughness", RenderPass::WidgetType::FLOAT, std::ref(m_materialData.roughness) },
        //{ "Use Normal Texture?", RenderPass::WidgetType::CHECKBOX, std::ref(m_materialData.useNormalTexture) }
    };
}

void GeometryPass::Update(ID3D11DeviceContext* deviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, bool useAlbedo, bool useNormal, bool useRoughness)
{
    m_materialData.viewMatrix = XMMatrixTranspose(viewMatrix);
    m_materialData.useAlbedoTexture = useAlbedo;
    m_materialData.useNormalTexture = useNormal;
    m_materialData.useRoughnessTexture = useRoughness;

    m_cameraData.worldMatrix= XMMatrixIdentity();
    m_cameraData.viewMatrix = XMMatrixTranspose(viewMatrix);
    m_cameraData.projectionMatrix = XMMatrixTranspose(projectionMatrix);

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    deviceContext->Map(m_materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    std::memcpy(mapped.pData, &m_materialData, sizeof(m_materialData));
    deviceContext->Unmap(m_materialBuffer, 0);

    deviceContext->Map(m_cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    std::memcpy(mapped.pData, &m_cameraData, sizeof(m_cameraData));
    deviceContext->Unmap(m_cameraBuffer, 0);
}

void GeometryPass::ClearGBuffer(ID3D11DeviceContext* deviceContext, float* clearColor)
{
    // Clear shadow
    deviceContext->ClearDepthStencilView(m_shadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
    
    // Clear the render target views
    deviceContext->ClearRenderTargetView(m_albedoRTV.Get(), clearColor);
    deviceContext->ClearRenderTargetView(m_normalRTV.Get(), clearColor);

    deviceContext->ClearDepthStencilView(m_dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void GeometryPass::Render(ID3D11DeviceContext* deviceContext, int indexCount)
{
    // Set the render targets (albedo and normal) along with the depth stencil view
    ID3D11RenderTargetView* renderTargets[2] = { m_albedoRTV.Get(), m_normalRTV.Get()};

    deviceContext->OMSetRenderTargets(2, renderTargets, m_dsv);

    deviceContext->VSSetConstantBuffers(0, 1, &m_cameraBuffer);
    deviceContext->PSSetConstantBuffers(0, 1, &m_materialBuffer);

    deviceContext->IASetInputLayout(m_layout);

	deviceContext->VSSetShader(m_vertexShader, nullptr, 0);
	deviceContext->PSSetShader(m_pixelShader, nullptr, 0);

	deviceContext->PSSetSamplers(0, 1, &m_sampleStateWrap);

	deviceContext->DrawIndexed(indexCount, 0, 0);
}


bool GeometryPass::InitializeShadow(ID3D11Device* device)
{
    ID3D10Blob* errorMessage;
    ID3D10Blob* shadowShaderBuffer;

    wchar_t shadowFilename[128];
    wcscpy_s(shadowFilename, 128, L"Shaders/shadow.vs");

    HRESULT result = D3DCompileFromFile(shadowFilename, nullptr, nullptr, "ShadowVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &shadowShaderBuffer, &errorMessage);
    if (FAILED(result))
    {
        if (errorMessage)
            OutputShaderErrorMessage(errorMessage, shadowFilename);
        return false;
    }

    result = device->CreateVertexShader(shadowShaderBuffer->GetBufferPointer(), shadowShaderBuffer->GetBufferSize(), nullptr, &m_shadowShader);
    if (FAILED(result))
        return false;
    
    D3D11_TEXTURE2D_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.Width = m_shadowMapSize;
    sd.Height = m_shadowMapSize;
    sd.MipLevels = 1;
    sd.ArraySize = 1;
    sd.Format = DXGI_FORMAT_R32_TYPELESS;
    sd.SampleDesc.Count = 1;
    sd.Usage = D3D11_USAGE_DEFAULT;
    sd.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

    ID3D11Texture2D* shadowTex = nullptr;
    result = device->CreateTexture2D(&sd, nullptr, &shadowTex);
    if (FAILED(result))
        return false;

    // for rendering into the shadow map
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
    ZeroMemory(&dsvDesc, sizeof(dsvDesc));
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    device->CreateDepthStencilView(shadowTex, &dsvDesc, &m_shadowDSV);

    // for sampling the shadow map in the lighting pass
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    device->CreateShaderResourceView(shadowTex, &srvDesc, &m_shadowSRV);

    shadowTex->Release();
    shadowTex = nullptr;

    D3D11_INPUT_ELEMENT_DESC pl[1];
    pl[0].SemanticName = "POSITION";
    pl[0].SemanticIndex = 0;
    pl[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    pl[0].InputSlot = 0;
    pl[0].AlignedByteOffset = 0;
    pl[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    pl[0].InstanceDataStepRate = 0;

    UINT numElements = sizeof(pl) / sizeof(pl[0]);

    result = device->CreateInputLayout(pl, numElements, shadowShaderBuffer->GetBufferPointer(), shadowShaderBuffer->GetBufferSize(), &m_shadowLayout);
    if (FAILED(result))
        return false;

    D3D11_BUFFER_DESC sbd;
    sbd.Usage = D3D11_USAGE_DYNAMIC;
    sbd.ByteWidth = sizeof(ShadowBufferType);
    sbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    sbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    sbd.MiscFlags = 0;
    sbd.StructureByteStride = 0;

    result = device->CreateBuffer(&sbd, nullptr, &m_shadowBuffer);
    if (FAILED(result))
        return false;

    m_shadowVp = { 0, 0, (float)m_shadowMapSize, (float)m_shadowMapSize, 0, 1 };

    return true;
}

bool GeometryPass::RenderShadow(ID3D11DeviceContext* deviceContext, int indexCount, XMVECTOR lightDirection)
{
    // Set up light source
    XMVECTOR target = XMVectorZero(); // TODO
    XMVECTOR lightPos = target - lightDirection * 30.0f;
    XMVECTOR up = XMVectorSet(0, 1, 0, 0);
    XMMATRIX lightView = XMMatrixLookAtLH(lightPos, target, up);

    XMMATRIX lightProj = XMMatrixOrthographicLH(60.0f, 60.0f, 0.01f, 80.0f); // TODO: Adjust

    m_lightViewProj = XMMatrixMultiply(lightView, lightProj);
    
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT result = deviceContext->Map(m_shadowBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
        return false;

    ShadowBufferType* dataPtr = (ShadowBufferType*)mappedResource.pData;

    dataPtr->lightViewProj = XMMatrixTranspose(m_lightViewProj);

    deviceContext->Unmap(m_shadowBuffer, 0);

    UINT bufferNumber = 0;

    deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_shadowBuffer);
    
    // Bind no color RTs, only the shadow DSV
    ID3D11RenderTargetView* nullRT = nullptr;
    deviceContext->OMSetRenderTargets(0, &nullRT, m_shadowDSV);

    // Set viewport to shadow map size

    deviceContext->RSSetViewports(1, &m_shadowVp);

    deviceContext->IASetInputLayout(m_shadowLayout);

    // Use a simple depth‐only shader: 
    //   VS: transform positions by lightViewProj matrix
    //   PS: (optional) empty or writes nothing
    deviceContext->VSSetShader(m_shadowShader, nullptr, 0);
    deviceContext->PSSetShader(nullptr, nullptr, 0);

    deviceContext->DrawIndexed(indexCount, 0, 0);

    return true;
}

void GeometryPass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, WCHAR* shaderFilename)
{
    char* compileErrors;
    unsigned long long bufferSize, i;
    std::ofstream fout;

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