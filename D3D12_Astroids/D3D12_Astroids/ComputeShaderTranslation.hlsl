struct BufTypeTrans
{
	float x, y, z;
};
RWStructuredBuffer<BufTypeTrans> BufferPosition : register(u2);
RWStructuredBuffer<BufTypeTrans> BufferDirection : register(u3);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	BufferPosition[DTid.x].x += BufferDirection[DTid.x].x * BufferDirection[DTid.x].z;
	BufferPosition[DTid.x].y += BufferDirection[DTid.x].y * BufferDirection[DTid.x].z;
	BufferPosition[DTid.x].z += BufferDirection[DTid.x].z * BufferDirection[DTid.x].z;
}