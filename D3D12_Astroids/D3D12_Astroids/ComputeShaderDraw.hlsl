
#define NROFOBJECTS 32
#define RADIUS 10.0f

RWTexture2D<float4> textureOut : register(u4);

struct BufTypeTrans
{
	float x, y, z;
};

RWStructuredBuffer<BufTypeTrans> BufferPosition : register(u2);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	float2 texPos = float2(DTid.x, DTid.y);
	float length = RADIUS;

	//float4 color = distance(pos0, texPos) < length ? 1.0f : 0.0f;
	float4 color = 0.0f;
	//float4 color = distance(pos0, texPos) < length || distance(pos1, texPos) < length ? 1.0f : 0.0f;
	//color = distance(pos1, texPos) < length ? 1.0f : 0.0f;

	float2 posPlayer = float2(BufferPosition[0].x, BufferPosition[0].y);

	if (distance(posPlayer, texPos) < length)
		color = float4(0.3f, 0.3f, 1.0f, 1.0f);
	else
	{
		for (int i = i; i < NROFOBJECTS; i++)
		{
			float2 posi = float2(BufferPosition[i].x, BufferPosition[i].y);
			if (distance(posi, texPos) < length)
				color = 1.0f;
		}
	}
	


	textureOut[uint2(DTid.x, DTid.y)].rgba = color;

	//textureOut[uint2(DTid.x, DTid.y)].rgba = float4(0.0f, 0.0f, 1.0f, 1.0f);
}