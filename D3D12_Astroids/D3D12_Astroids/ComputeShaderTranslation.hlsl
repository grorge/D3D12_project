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
		float3 pos = BufferPosition[0];
		float3 bullet = BufferBulletPosition[BufferBulletPosition[0].x];

		if (BufferBulletPosition[0].y == 1.0f && distance(pos.y, bullet.y) >= 100.0f)
		{
			BufferBulletPosition[0].x %= 127;
			BufferBulletPosition[0].x++;
			BufferBulletDirection[BufferBulletPosition[0].x].z = 4.0f;
			BufferBulletPosition[BufferBulletPosition[0].x] = pos;
		}
	}

	BufferPosition[DTid.x].x += BufferDirection[DTid.x].x * BufferDirection[DTid.x].z * SPEED;
	BufferPosition[DTid.x].y += BufferDirection[DTid.x].y * BufferDirection[DTid.x].z * SPEED;
	//BufferPosition[DTid.x].z += BufferDirection[DTid.x].z * BufferDirection[DTid.x].z;

	
}