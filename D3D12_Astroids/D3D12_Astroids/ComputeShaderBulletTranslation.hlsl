RWStructuredBuffer<float3> BufferPosition : register(u4);
RWStructuredBuffer<float3> BufferDirection : register(u5);

#define SPEED 1.0f

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	int index = DTid.x + 1;

	float3 dir = BufferDirection[index];
	float3 pos = BufferPosition[index];

	pos.xy += dir.xy * dir.z * SPEED;

	if (BufferPosition[index].y < -5.0f)
	{
		pos.xy	= -100.0f;
		pos.z	= 1.0f;

		dir.z = 0.0f;
		BufferDirection[index] = dir;
	}

	BufferPosition[index] = pos;
}