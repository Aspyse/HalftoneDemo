cbuffer MatrixBuffer : register(b0)
{
    float4x4 worldMatrix;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

struct VertexInputType
{
    float4 position : POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL;
};

PixelInputType GeometryVertexShader(VertexInputType input)
{
    PixelInputType output;
    
    input.position.w = 1.0f;

    //output.position = mul(input.position, worldMatrix);

    output.position = mul(input.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.normal = normalize(mul(input.normal, (float3x3)viewMatrix));
    //output.normal = input.normal;

    output.uv = input.uv;

    return output;
}