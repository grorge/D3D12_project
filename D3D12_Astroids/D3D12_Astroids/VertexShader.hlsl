struct VSIn
{
	float3 pos		: POSITION;
	float3 color	: COLOR;
};

struct VSOut
{
	float4 pos		: SV_POSITION;
	float4 color	: COLOR;
};

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
	output.pos		= float4( input.pos, 1.0f) + translation[instance];
	//output.pos		= float4( input.pos, 1.0f );
	output.color	= color[instance];
	//output.color	= float4(R, G, B, A);

	return output;
}