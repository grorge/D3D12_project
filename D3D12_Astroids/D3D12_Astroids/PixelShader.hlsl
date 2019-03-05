SamplerState samplerState : register (s0);
Texture2D texture2d : register (t2);

struct VSOut
{
	float4 pos		: SV_POSITION;
	float2 uv		: TEXCOORD0;
};

float4 main( VSOut input ) : SV_TARGET0
{
	return texture2d.Sample(samplerState, input.uv);
	//return input.color;
}