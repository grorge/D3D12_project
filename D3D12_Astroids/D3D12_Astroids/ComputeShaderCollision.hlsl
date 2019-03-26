
#define NROFOBJECTS 256 * 2
#define NROFBULLETS 128
#define RADIUS 10.0f
#define RADIUS_BULLET 5.0f

struct BufTypeTrans
{
	float x, y, z;
};
RWStructuredBuffer<BufTypeTrans> BufferPosition : register(u2);
RWStructuredBuffer<BufTypeTrans> BufferDirection : register(u3);

RWStructuredBuffer<BufTypeTrans> BufferBulletPosition: register(u4);

#define SCREEN_WIDTH 1600.0f
#define SCREEN_HEIGHT 900.0f

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	int index = DTid.x + 1; // Don't check collision for player

	BufTypeTrans tempObj = BufferPosition[index];
	float3 thisObj = float3(tempObj.x, tempObj.y, tempObj.z);

	BufTypeTrans tempDir = BufferDirection[index];
	float3 dir = float3(tempDir.x, tempDir.y, tempDir.z);

	//dir.x *= (thisObj.x < 0.0f || thisObj.x > SCREEN_WIDTH)			?  -1.0f : 1.0f;
	//dir.y *= (thisObj.y < 0.0f || thisObj.y > SCREEN_HEIGHT)		?  -1.0f : 1.0f;

	dir.x = (thisObj.x < 0.0f)			?  1.0f : dir.x;
	dir.x = (thisObj.x > SCREEN_WIDTH)  ? -1.0f : dir.x;

	dir.y = (thisObj.y < 0.0f)			?  1.0f : dir.y;
	thisObj.y = (thisObj.y > SCREEN_HEIGHT) ? -1.0f : thisObj.y;

	float3 thatObj = { BufferPosition[0].x, BufferPosition[0].y, 1.0f };

	float dist = distance(thisObj, thatObj);

	if (dist <= RADIUS * 2.0f)
	{
		dir.x *= -1.0f;
		dir.y *= -1.0f;
		//dir.z *= 1.01f;

		BufferPosition[0].z = -1.0f;
	}

	for (int i = 1; i < NROFBULLETS; i++)
	{
		if (BufferBulletPosition[i].z != -1.0f ||true)
		{
			float3 thatObj = { BufferBulletPosition[i].x, BufferBulletPosition[i].y, 1.0f };

			float dist = distance(thisObj, thatObj);

			if (dist <= RADIUS_BULLET + RADIUS)
			{
				//dir.x *= -1.0f;
				//dir.y *= -1.0f;
				dir.z = 0.0f;

				BufferPosition[index].x = -100.0f; 
				BufferBulletPosition[i].x = -100.0f;
			}
		}
	}

	BufTypeTrans newDir;
	newDir.x = dir.x;
	newDir.y = dir.y;
	newDir.z = dir.z;

	BufferPosition[index].y = thisObj.y;

	BufferDirection[index] = newDir;
}