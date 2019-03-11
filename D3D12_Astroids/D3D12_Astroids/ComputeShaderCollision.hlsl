
#define NROFOBJECTS 32
#define RADIUS 20.0f

struct BufTypeTrans
{
	float x, y, z;
};
RWStructuredBuffer<BufTypeTrans> BufferPosition : register(u2);

[numthreads(32, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	float3 thisObj = { BufferPosition[DTid.x].x, BufferPosition[DTid.x].y, 1.0f };
	for (int i = 0; i < NROFOBJECTS; i++)
	{
		if (DTid.x != i)
		{
			float3 thatObj = { BufferPosition[i].x, BufferPosition[i].y, 1.0f };

			float dist = distance(thisObj, thatObj);

			if (dist <= RADIUS)
			{
				BufferPosition[DTid.x].x = -1.0f;
			}
		}
	}
}