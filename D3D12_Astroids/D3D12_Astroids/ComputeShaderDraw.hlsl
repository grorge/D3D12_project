struct BufTypeFloat4
{
	float x, y, z, w;
};

RWStructuredBuffer<BufTypeFloat4> BufferOut : register(u2);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	BufferOut[DTid.x].x += 5.0f;
	BufferOut[DTid.x].y = 4.0f;
	BufferOut[DTid.x].z = 8.0f;
}