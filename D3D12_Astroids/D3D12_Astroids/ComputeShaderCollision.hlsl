#define NROFBULLETS 128
#define RADIUS 10.0f
#define RADIUS_BULLET 5.0f

RWStructuredBuffer<float3> BufferPosition : register(u2);
RWStructuredBuffer<float3> BufferDirection : register(u3);

RWStructuredBuffer<float3> BufferBulletPosition: register(u4);

#define SCREEN_WIDTH 1600.0f
#define SCREEN_HEIGHT 900.0f

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	int index = DTid.x + 1; // Don't check collision for player

	float3 thatObj	= BufferPosition[0];
	float3 thisObj	= BufferPosition[index];
	float3 dir		= BufferDirection[index];

	dir.x = (thisObj.x < 0.0f)			?  1.0f : dir.x;
	dir.x = (thisObj.x > SCREEN_WIDTH)  ? -1.0f : dir.x;

	dir.y		= (thisObj.y < 0.0f)				?  1.0f : dir.y;
	thisObj.y	= (thisObj.y > SCREEN_HEIGHT)		? -1.0f : thisObj.y;

	if (distance(thisObj, thatObj) <= RADIUS * 2.0f)
	{
		dir.x *= -1.0f;
		dir.y *= -1.0f;

		BufferPosition[0].z = -1.0f;
	}

	for (int j = 0; j < 1; j++)
	{
		for (int i = 1; i < NROFBULLETS; i++)
		{
			float3 thatObj = BufferBulletPosition[i];

			float dist = distance(thisObj, thatObj);

			if (dist <= RADIUS_BULLET + RADIUS)
			{
				dir.z = 0.0f;

				thisObj.y = -100.0f;
				BufferBulletPosition[i].x = -100.0f;
			}
		}
	}

	BufferPosition[index].y = thisObj.y;
	BufferDirection[index] = dir;
}