
#define NROFOBJECTS 32
#define RADIUS 0.5f

struct BufTypeTrans
{
	float x, y, z;
};
RWStructuredBuffer<BufTypeTrans> BufferTrans : register(u2);

[numthreads(32, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	float3 thisObj = { BufferTrans[DTid.x].x, BufferTrans[DTid.x].y, 1.0f };
	for (int i = 0; i < NROFOBJECTS; i++)
	{
		if (DTid.x != i)
		{
			float3 thatObj = { BufferTrans[i].x, BufferTrans[i].y, 1.0f };

			float dist = distance(thisObj, thatObj);

			if (dist <= RADIUS)
			{
				BufferTrans[DTid.x].z = -1.0f;
			}
		}
	}
}