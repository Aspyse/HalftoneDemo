struct PixelInputType
{
	float2 uv : TEXCOORD0;
};

PixelInputType BaseVertexShader(uint vertexID : SV_VertexID)
{
    PixelInputType output;
    
    float2 pos = float2((vertexID << 1) & 2, vertexID & 2);
    output.uv = pos;
    
    return output;
}
