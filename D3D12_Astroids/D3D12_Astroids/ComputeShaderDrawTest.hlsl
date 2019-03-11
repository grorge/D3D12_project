
#define NROFOBJECTS 64
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
	float2 index	= float2(DTid.x, DTid.y);
	float length		= RADIUS;

	float2 pos = float2(
		BufferPosition[index.x].x,
		BufferPosition[index.y].y);

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


	//textureOut[uint2(DTid.x, DTid.y)].rgba = color;
}