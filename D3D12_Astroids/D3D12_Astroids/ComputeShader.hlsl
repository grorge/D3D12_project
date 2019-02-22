
struct BufType
{
	float x, y, z;
};

//StructuredBuffer<BufType> Buffer0 : register(t0);
//StructuredBuffer<BufType> Buffer1 : register(t1);
RWStructuredBuffer<BufType> BufferOut : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	/*BufferOut[DTid.x].x = 0.0f;
	BufferOut[DTid.x].y = 0.0f;
	BufferOut[DTid.x].z = 0.0f;*/
}