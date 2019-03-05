Texture2D texture2d : register (t0);

struct VSOut
{
	float4 pos		: SV_POSITION;
	float2 uv		: TEXCOORD0;
};

float4 main( VSOut input ) : SV_TARGET0
{
	int3 dank = int3(input.pos.x, input.pos.y, 0);
	//int3 dank = int3(399, 224, 0);
	return texture2d.Load(dank);
	//return float4(1.0f, 0.0f, 0.0f, 1.0f);
}