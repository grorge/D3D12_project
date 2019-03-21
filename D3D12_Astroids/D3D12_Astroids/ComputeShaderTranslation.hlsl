RWStructuredBuffer<float3> BufferPosition : register(u2);
RWStructuredBuffer<float3> BufferDirection : register(u3);

#define SPEED 0.05f

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	float3 pos = BufferPosition[DTid.x];
	float3 dir = BufferDirection[DTid.x];

	pos.x += dir.x * dir.z * SPEED;
	pos.y += dir.y * dir.z * SPEED;

	if (pos.x < 20.0f && DTid.x == 0)
		pos.x = 20.0f;

	if (pos.x > 1260.0f && DTid.x == 0)
		pos.x = 1260.0f;

	if (pos.y < 20.0f && DTid.x == 0)
		pos.y = 20.0f;
	
	if (pos.y > 700.0f && DTid.x == 0)
		pos.y = 700.0f;

	BufferPosition[DTid.x] = pos;

	//BufferPosition[DTid.x].x += BufferDirection[DTid.x].x * BufferDirection[DTid.x].z * SPEED;
	//BufferPosition[DTid.x].y += BufferDirection[DTid.x].y * BufferDirection[DTid.x].z * SPEED;
	//BufferPosition[DTid.x].z += BufferDirection[DTid.x].z * BufferDirection[DTid.x].z;
}