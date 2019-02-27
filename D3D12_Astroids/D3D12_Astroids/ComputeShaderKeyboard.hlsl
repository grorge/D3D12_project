#define key_W 0x57
#define key_A 0x41
#define key_S 0x53
#define key_D 0x44
#define key_Space 0x20


struct BufTypeFloat4
{
	float x, y, z, w;
};
struct BufTypeIntArray
{
	unsigned int arr[256];
};

//StructuredBuffer<BufType> Buffer0 : register(t0);
//StructuredBuffer<BufType> Buffer1 : register(t1);
RWStructuredBuffer<BufTypeFloat4> BufferOut : register(u0);
RWStructuredBuffer<BufTypeIntArray> BufferOutKeyboard : register(u1);

[numthreads(256, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	if (BufferOutKeyboard[0].arr[DTid.x] != 0)
		switch (DTid.x)
		{
		case key_W:
			BufferOut[0].x += 20.0f;
			break;
		case key_S:
			BufferOut[0].x += 200.0f;
			break;
		case key_A:
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
	//if (BufferOutKeyboard[0].arr[DTid.x] != 0)
	//	BufferOut[0].x += 20.0f;
	//if (BufferOutKeyboard[DTid.x].arr[key_A] != 0)
	//	BufferOut[DTid.x].x += 20.0f;
	//if (BufferOutKeyboard[DTid.x].arr[key_S] != 0)
	//	BufferOut[DTid.x].x += 200.0f;
	//if (BufferOutKeyboard[DTid.x].arr[key_D] != 0)
	//	BufferOut[DTid.x].x += 2000.0f;
	//if (BufferOutKeyboard[DTid.x].arr[key_Space] != 0)
	//	BufferOut[DTid.x].x += 20000.0f;

}