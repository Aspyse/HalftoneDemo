cbuffer LightCamBuffer : register(b0)
{
	float4x4 lightViewProj;
}

struct VertexInputType
{
	float3 pos : POSITION;
};

struct VertexOutputType
{
	float4 posLS : SV_POSITION;
};

VertexOutputType ShadowVertexShader(VertexInputType input)
{
	VertexOutputType output;
	output.posLS = mul(float4(input.pos,1), lightViewProj);

	return output;
}