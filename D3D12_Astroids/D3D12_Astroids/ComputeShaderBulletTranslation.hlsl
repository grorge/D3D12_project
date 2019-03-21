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
	BufferPosition[DTid.x].x += BufferDirection[DTid.x].x * BufferDirection[DTid.x].z * SPEED;
	BufferPosition[DTid.x].y += BufferDirection[DTid.x].y * BufferDirection[DTid.x].z * SPEED;
	//BufferPosition[DTid.x].z += BufferDirection[DTid.x].z * BufferDirection[DTid.x].z;
}