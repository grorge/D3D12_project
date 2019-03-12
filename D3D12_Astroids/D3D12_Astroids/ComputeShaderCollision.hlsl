
#define NROFOBJECTS 256
#define RADIUS 10.0f

struct BufTypeTrans
{
	float x, y, z;
};
RWStructuredBuffer<BufTypeTrans> BufferPosition : register(u2);
RWStructuredBuffer<BufTypeTrans> BufferDirection : register(u3);

#define SCREEN_WIDTH 960.0f
#define SCREEN_HEIGHT 540.0f

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	int index = DTid.x + 1; // Don't collide with player

	BufTypeTrans tempObj = BufferPosition[index];
	float3 thisObj = float3(tempObj.x, tempObj.y, tempObj.z);

	BufTypeTrans tempDir = BufferDirection[index];
	float3 dir = float3(tempDir.x, tempDir.y, tempDir.z);

	dir.x = (thisObj.x < 0.0f)			?  1.0f : dir.x;
	dir.x = (thisObj.x > SCREEN_WIDTH)  ? -1.0f : dir.x;

	dir.y = (thisObj.y < 0.0f)			?  1.0f : dir.y;
	dir.y = (thisObj.y > SCREEN_HEIGHT) ? -1.0f : dir.y;

	BufTypeTrans newDir;
	newDir.x = dir.x;
	newDir.y = dir.y;
	newDir.z = dir.z;

	BufferDirection[index] = newDir;


	//if (thisObj.x < 0.0f)
	//{
	//	BufferDirection[DTid.x].x = 1.0f;
	//	//BufferPosition[DTid.x].x = 0.0f;
	//}
	//else if (thisObj.x > 960.0f)
	//{
	//	BufferDirection[DTid.x].x = -1.0f;
	//	//BufferPosition[DTid.x].x = 960.0f;
	//}
	//else if (thisObj.y < 0.0f)
	//{
	//	BufferDirection[DTid.x].y = 1.0f;
	//	//BufferPosition[DTid.x].y = 0.0f;
	//}
	//else if (thisObj.y > 540.0f)
	//{
	//	BufferDirection[DTid.x].y = -1.0f;
	//	//BufferPosition[DTid.x].y = 540.0f;
	//}
	/*else
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
		}*/
}