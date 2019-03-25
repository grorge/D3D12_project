#define NROFOBJECTS 256 * 8
#define RADIUS 10.0f

RWTexture2D<float4> textureOut : register(u7);

struct BufTypeTrans
{
	float x, y, z;
};

RWStructuredBuffer<float3> BufferPosition : register(u2);

#define BLOCK_WIDTH 256 * 8

//[numthreads(BLOCK_WIDTH, 1, 1)]
[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	float index = DTid.x + DTid.y * BLOCK_WIDTH;
	float length = RADIUS;

	float4 color = index == 0 ?
		float4(0.1f, 0.7f, 1.0f, 1.0f) :
		float4(1.0f, 0.1f, 0.1f, 1.0f);

	float2 pos = BufferPosition[index].xy;

	unsigned int yMin = pos.y - length;
	unsigned int yMax = pos.y + length + 1;

	unsigned int xMin = pos.x - length;
	unsigned int xMax = pos.x + length + 1;

	for (unsigned int y = yMin; y < yMax; y++)
	{
		for (unsigned int x = xMin; x < xMax; x++)
		{
			if (distance(float2(x, y), pos) < length)
			{
				textureOut[uint2(x, y)].rgba = color;
			}
		}
	}
}