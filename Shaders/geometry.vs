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
    float4 tangent : TANGENT;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 albedoUV : TEXCOORD0;
    float2 normalUV : TEXCOORD1;
    float2 roughUV : TEXCOORD2;
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

    float3 N = normalize(input.normal);
    
    float3 T = input.tangent.xyz;
    T = normalize(T - N * dot(N, T));
    
    float3 B = cross(N, T) * input.tangent.w;
    B = normalize(B);
    
    
    output.normal = N;
    output.tangent = T;
    output.binormal = B;

    // TODO
    output.albedoUV = input.uv;
    output.normalUV = input.uv;
    output.roughUV = input.uv;

    return output;
}