
#define NROFOBJECTS 64
#define RADIUS 10.0f

struct BufTypeTrans
{
	float x, y, z;
};
RWStructuredBuffer<BufTypeTrans> BufferPosition : register(u2);
RWStructuredBuffer<BufTypeTrans> BufferDirection : register(u3);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	float3 thisObj = { BufferPosition[DTid.x].x, BufferPosition[DTid.x].y, 1.0f };

	if (thisObj.x < 0.0f)
	{
		BufferDirection[DTid.x].x *= -1.0f;
		BufferPosition[DTid.x].x = 0.0f;
	}
	else if (thisObj.x > 960.0f)
	{
		BufferDirection[DTid.x].x *= -1.0f;
		BufferPosition[DTid.x].x = 960.0f;
	}
	else if (thisObj.y < 0.0f)
	{
		BufferDirection[DTid.x].y *= -1.0f;
		BufferPosition[DTid.x].y = 0.0f;
	}
	else if (thisObj.y > 540.0f)
	{
		BufferDirection[DTid.x].y *= -1.0f;
		BufferPosition[DTid.x].y = 540.0f;
	}
	else
		for (int i = 0; i < NROFOBJECTS; i++)
		{
			if (DTid.x != i)
			{
				float3 thatObj = { BufferPosition[i].x, BufferPosition[i].y, 1.0f };

				float dist = distance(thisObj, thatObj);

				if (dist <= RADIUS * 2.0f)
				{
					BufferDirection[DTid.x].x *= -1.0f;
					BufferDirection[DTid.x].y *= -1.0f;
					BufferDirection[DTid.x].z *= 1.1f;
				}
			}
		}
}