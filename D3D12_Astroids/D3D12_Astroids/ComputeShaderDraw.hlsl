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

	float4 color = index == 0 ?
		float4(1.0f, 0.0f, 0.0f, 1.0f) :
		float4(1.0f, 1.0f, 1.0f, 1.0f);

	float2 pos = float2(
		BufferPosition[index].x,
		BufferPosition[index].y);

	int yMin = pos.y - length;
	int yMax = pos.y + length + 1;

	int xMin = pos.x - length;
	int xMax = pos.x + length + 1;

	for (unsigned int y = yMin; y < yMax; y++)
	{
		for (unsigned int x = xMin; x < xMax; x++)
		{
			textureOut[uint2(x, y)].rgba = 
				distance(float2(x, y), pos) < length ?
					color : textureOut[uint2(x, y)].rgba;
		}
	}
}