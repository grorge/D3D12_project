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

[numthreads(3, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	BufferOut[DTid.x].x = BufferTrans[DTid.x].x;
	BufferOut[DTid.x].y = BufferTrans[DTid.x].y;
	BufferOut[DTid.x].z = BufferTrans[DTid.x].z;
	//BufferOut[DTid.x].x = 0.5f;
	//BufferOut[DTid.x].y = 0.5f;
	//BufferOut[DTid.x].z = 0.5f;

	//BufferOut[DTid.x].x = DTid.x;
	//BufferOut[DTid.x].y = BufferTrans[DTid.x].y;
	//BufferOut[DTid.x].z = BufferTrans[DTid.x].z;
}