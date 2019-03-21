
RWTexture2D<float4> textureOut : register(u8);
//RWStructuredBuffer<float4> textureOut : register(u8);


[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	textureOut[uint2(DTid.x, DTid.y)].rgba = 0.1f;
	//textureOut[DTid.x + DTid.y * 1600].rgba = 1.0f;
}