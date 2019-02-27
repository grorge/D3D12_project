struct BufTypeFloat4
{
	float x, y, z, w;
};

//StructuredBuffer<BufType> Buffer0 : register(t0);
//StructuredBuffer<BufType> Buffer1 : register(t1);
RWStructuredBuffer<BufTypeFloat4> BufferOut : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	BufferOut[DTid.x].x += 2.0f;
	BufferOut[DTid.x].y = 4.0f;
	BufferOut[DTid.x].z = 8.0f;
}