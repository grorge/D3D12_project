struct BufTypeTrans
{
	float x, y, z;
};
RWStructuredBuffer<BufTypeTrans> BufferPosition : register(u4);
RWStructuredBuffer<BufTypeTrans> BufferDirection : register(u5);

#define SPEED 1.0f

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	int index = DTid.x + 1;
	BufferPosition[index].x += BufferDirection[index].x * BufferDirection[index].z * SPEED;
	BufferPosition[index].y += BufferDirection[index].y * BufferDirection[index].z * SPEED;

	if (BufferPosition[index].y < -5.0f)
	{
		BufferPosition[index].z = 1.0f;
		BufferPosition[index].x = -100.0f;
		BufferPosition[index].y = 100.0f;
		BufferDirection[index].z = 0.0f;
	}
	
	//BufferPosition[DTid.x].z += BufferDirection[DTid.x].z * BufferDirection[DTid.x].z;
}