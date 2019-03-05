#define key_W 0
#define key_A 1
#define key_S 2
#define key_D 3
#define key_Space 4


struct BufTypeFloat4
{
	float x, y, z, w;
};
struct BufTypeIntArray
{
	unsigned int arr[32];
};

RWStructuredBuffer<BufTypeFloat4> BufferOut : register(u0);
RWStructuredBuffer<BufTypeIntArray> BufferOutKeyboard : register(u1);

[numthreads(32, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	if (BufferOutKeyboard[0].arr[DTid.x] != 0)
		switch (DTid.x)
		{
		case key_W:
			BufferOut[0].x += 20.0f;
			break;
		case key_A:
			BufferOut[0].x += 200.0f;
			break;
		case key_S:
			BufferOut[0].x += 2000.0f;
			break;
		case key_D:
			BufferOut[0].x += 20000.0f;
			break;
		case key_Space:
			BufferOut[0].x += 200000.0f;
			break;
		default:
			break;
		}
}