RWTexture2D<float4> textureOut : register(u3);

struct BufTypeTrans
{
	float x, y, z;
};

RWStructuredBuffer<BufTypeTrans> BufferTrans : register(u2);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	float2 pos0 = float2(BufferTrans[0].x, BufferTrans[0].y);
	float2 pos1 = float2(BufferTrans[1].x, BufferTrans[1].y);
	
	float2 texPos = float2(DTid.x, DTid.y);
	float length = 10.0f;

	//float4 color = distance(pos0, texPos) < length ? 1.0f : 0.0f;
	float4 color = distance(pos0, texPos) < length || distance(pos1, texPos) < length ? 1.0f : 0.0f;
	//color = distance(pos1, texPos) < length ? 1.0f : 0.0f;


	textureOut[uint2(DTid.x, DTid.y)].rgba = color;

	//textureOut[uint2(DTid.x, DTid.y)].rgba = float4(0.0f, 0.0f, 1.0f, 1.0f);
}