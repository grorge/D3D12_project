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
struct BufTypeTrans
{
	float x, y, z;
};

RWStructuredBuffer<BufTypeFloat4> BufferOut : register(u0);
RWStructuredBuffer<BufTypeIntArray> BufferInKeyboard : register(u1);
RWStructuredBuffer<BufTypeTrans> BufferTrans : register(u2);

[numthreads(32, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	if (BufferInKeyboard[0].arr[DTid.x] != 0)
		switch (DTid.x)
		{
		case key_W:
			BufferTrans[0].y += 1.0f;
			break;
		case key_A:
			BufferTrans[0].x += -1.0f;
			break;
		case key_S:
			BufferTrans[0].y += -1.0f;
			break;
		case key_D:
			BufferTrans[0].x += 1.0f;
			break;
		case key_Space:
			BufferTrans[0].z += 2.0f;
			break;
		default:
			break;
		}

}