
#define RADIUS 5.0f

RWTexture2D<float4> textureOut : register(u7);

RWStructuredBuffer<float3> BufferPosition : register(u4);

#define BLOCK_WIDTH 128

[numthreads(BLOCK_WIDTH, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	float index = DTid.x;
	float length = RADIUS;

	float4 color = float4(0.1f, 1.0f, 0.1f, 1.0f);

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