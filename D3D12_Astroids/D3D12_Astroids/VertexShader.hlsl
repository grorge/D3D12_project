struct VSIn
{
	float3 pos		: POSITION;
	float2 uv		: TEXCOORD0;
	//float3 color	: COLOR;
};

struct VSOut
{
	float4 pos		: SV_POSITION;
	float2 uv		: TEXCOORD0;
	//float4 color	: COLOR;
};

struct BufTypeFloat4
{
	float x, y, z, w;
};

RWStructuredBuffer<BufTypeFloat4> BufferOut : register(u0);

cbuffer ColorBuffer : register(b0)
{
	//float R, G, B, A;
	float4 color[512];
}
cbuffer TranslationBuffer : register(b1)
{
	float4 translation[512];
}

VSOut main( VSIn input, 
	uint index : SV_VertexID ,
	uint instance : SV_InstanceID)
{
	VSOut output	= (VSOut)0;
	//output.pos		= float4( input.pos, 1.0f) + translation[instance];
	output.pos		= float4( input.pos, 1.0f );
	
	//output.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	//output.color	= color[instance];
	//output.color = float4(input.color, 1.0f);
	//output.color	= float4(R, G, B, A);
	output.uv = input.uv;

	return output;
}