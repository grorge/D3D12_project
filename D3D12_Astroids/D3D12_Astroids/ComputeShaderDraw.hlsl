RWTexture2D<float4> textureOut : register(u2);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	float2 pos = float2(100.0f, 100.0f);
	float2 texPos = float2(DTid.x, DTid.y);
	float length = 10.0f;

	float4 color = distance(pos, texPos) < length ? 1.0f : 0.0f;

	textureOut[uint2(DTid.x, DTid.y)].rgba = color;

	//textureOut[uint2(DTid.x, DTid.y)].rgba = float4(0.0f, 0.0f, 1.0f, 1.0f);
}