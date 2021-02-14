struct VertexInput
{
	float2 pos : POSITION0;
	float2 uv : TEXCOORD0;
	float4 color : COLOR0;
};

struct VertexOutput
{
	float2 uv : TEXCOORD0;
	float4 color : COLOR0;
	float4 pos : SV_POSITION;
};

[[vk::push_constant]]
struct PushConstants
{
	float2 scale;
	float2 translate;
} pushConstants;

VertexOutput main(VertexInput input)
{
	VertexOutput output;
	output.uv = input.uv;
	output.color = input.color;
	output.pos = float4(input.pos * pushConstants.scale + pushConstants.translate, 0.0, 1.0);
	return output;
}