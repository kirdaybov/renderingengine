
struct VSInput
{
	float3 pos : POSITION0;
	float2 texCoord : TEXCOORD0;
};

struct VSOutput
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

VSOutput main(VSInput input)
{
	VSOutput output;
	output.pos = float4(input.pos, 1.0f);
	output.texCoord = input.texCoord;
	return output;
}