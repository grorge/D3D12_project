struct BufTypeFloat4
{
	float x, y, z, w;
};
struct BufTypeTrans
{
	float x, y, z;
};

//StructuredBuffer<BufType> Buffer0 : register(t0);
//StructuredBuffer<BufType> Buffer1 : register(t1);
RWStructuredBuffer<BufTypeFloat4> BufferOut : register(u0);
RWStructuredBuffer<BufTypeTrans> BufferTrans : register(u2);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	BufferOut[DTid.x].x = BufferTrans[DTid.x].x;
	BufferOut[DTid.x].y = BufferTrans[DTid.x].y;
	BufferOut[DTid.x].z = BufferTrans[DTid.x].z;
}