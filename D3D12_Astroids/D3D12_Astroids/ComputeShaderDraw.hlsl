#define NROFOBJECTS 256
#define RADIUS 10.0f

RWTexture2D<float4> textureOut : register(u4);

struct BufTypeTrans
{
	float x, y, z;
};

RWStructuredBuffer<BufTypeTrans> BufferPosition : register(u2);

#define BLOCK_WIDTH 256

[numthreads(BLOCK_WIDTH, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	float index = DTid.x + DTid.y * BLOCK_WIDTH;
	float length = RADIUS;

	float2 pos = float2(
		BufferPosition[index].x,
		BufferPosition[index].y);

	float4 color = 0.0f;

	int yMin = pos.y - length;
	int yMax = pos.y + length;

	int xMin = pos.x - length;
	int xMax = pos.x + length;

	for (unsigned int y = yMin; y < yMax; y++)
	{
		for (unsigned int x = xMin; x < xMax; x++)
		{
			textureOut[uint2(x, y)].rgba =
				distance(float2(x, y), pos) < length ? 1.0f : 0.0f;
		}
	}
}