RWTexture2D<float4> textureOut : register(u2);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	textureOut[uint2(DTid.x, DTid.y)].rgba = float4(0.0f, 0.0f, 1.0f, 1.0f);
}