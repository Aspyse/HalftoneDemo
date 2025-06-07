cbuffer MatrixBuffer : register(b0)
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

struct VertexInputType
{
    float3 position : POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
};

PixelInputType GeometryVertexShader(VertexInputType input)
{
    PixelInputType output;
    
    float4 pos = float4(input.position, 1);

    output.position = mul(pos, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    //output.normal = normalize(mul(input.normal, (float3x3)viewMatrix));
    output.normal = normalize(input.normal);
    
    output.tangent = normalize(input.tangent);
    
    output.binormal = normalize(input.binormal);

    output.uv = input.uv;

    return output;
}