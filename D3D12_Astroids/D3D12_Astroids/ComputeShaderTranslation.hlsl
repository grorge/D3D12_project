struct BufTypeTrans
{
	float x, y, z;
};
RWStructuredBuffer<float3> BufferPosition : register(u2);
RWStructuredBuffer<float3> BufferDirection : register(u3);

RWStructuredBuffer<float3> BufferBulletPosition: register(u4);
RWStructuredBuffer<float3> BufferBulletDirection: register(u5);

#define SPEED 1.0f

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	if (DTid.x == 0)
	{
		float3 bulletHeader = BufferBulletPosition[0];

		float3 pos = BufferPosition[0];
		float3 bullet = BufferBulletPosition[bulletHeader.x];

		if (bulletHeader.y == 1.0f && distance(pos.y, bullet.y) >= 100.0f)
		{
			bulletHeader.x %= 127;
			bulletHeader.x++;

			BufferBulletDirection[bulletHeader.x].z = 4.0f;
			BufferBulletPosition[bulletHeader.x]	= pos;

			BufferBulletPosition[0] = bulletHeader;
		}
	}

	float3 dir = BufferDirection[DTid.x];
	float3 pos = BufferPosition[DTid.x];

	pos.xy += dir.xy * dir.z * SPEED;

	BufferPosition[DTid.x] = pos;
}