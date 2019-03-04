struct BufTypeFloat4
{
	float x, y, z, w;
};
struct BufTypeIntArray
{
	unsigned int arr[256];
};

RWStructuredBuffer<BufTypeFloat4> BufferOut : register(u0);
RWStructuredBuffer<BufTypeIntArray> BufferOutKeyboard : register(u1);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	BufferOut[DTid.x].x += 11.0f;
	BufferOut[DTid.x].y += 22.0f;
	BufferOut[DTid.x].z += 33.0f;
	BufferOut[DTid.x].w += 44.0f;
}