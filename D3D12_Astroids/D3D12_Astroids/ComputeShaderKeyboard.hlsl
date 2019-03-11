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
//RWStructuredBuffer<BufTypeTrans> BufferPosition : register(u2);
RWStructuredBuffer<BufTypeTrans> BufferDirection : register(u3);

[numthreads(32, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	//if (BufferInKeyboard[0].arr[DTid.x] != 0)
		switch (DTid.x)
		{
		case key_W:
		case key_S:
			if (BufferInKeyboard[0].arr[key_W] != 0)
				BufferDirection[0].y = -1.0f;
			else if (BufferInKeyboard[0].arr[key_S] != 0)
				BufferDirection[0].y = 1.0f;
			else
				BufferDirection[0].y = 0.0f;
			break;
		case key_A:
		case key_D:
			if (BufferInKeyboard[0].arr[key_A] != 0)
				BufferDirection[0].x = -1.0f;
			else if (BufferInKeyboard[0].arr[key_D] != 0)
				BufferDirection[0].x = 1.0f;
			else
				BufferDirection[0].x = 0.0f;
			break;
		//case key_S:
		//	BufferDirection[0].y = 1.0f;
		//	break;
		//case key_D:
		//	BufferDirection[0].x = 1.0f;
		//	break;
		case key_Space:
			if (BufferInKeyboard[0].arr[key_Space] != 0)
				BufferDirection[0].z = 2.0f;
			else
				BufferDirection[0].z = 1.0f;
			break;
		default:
			break;
		}

	
	//switch (DTid.x)
	//{
	//case key_W:
	//	if (BufferInKeyboard[0].arr[key_S] == 0)
	//		BufferDirection[0].y = 0.0f;
	//	break;
	//case key_A:
	//	if (BufferInKeyboard[0].arr[key_D] == 0)
	//		BufferDirection[0].x = 0.0f;
	//	break;
	//case key_S:
	//	if (BufferInKeyboard[0].arr[key_W] == 0)
	//		BufferDirection[0].y = 0.0f;
	//	break;
	//case key_D:
	//	if (BufferInKeyboard[0].arr[key_A] == 0)
	//		BufferDirection[0].x = 0.0f;
	//	break;
	//case key_Space:
	//	BufferDirection[0].z = 2.0f;
	//	break;
	//default:
	//	break;
	//}
}