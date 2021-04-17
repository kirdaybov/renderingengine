
struct UniformBufferObject
{
	float4x4 model;
	float4x4 view;
	float4x4 proj;
	float3 lightDir;
} ubo;

struct VSInput
{
	float3 pos : POSITION0; //[[vk::location(0)]]
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
	output.pos = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.pos, 1.0f))));
	output.texCoord = input.texCoord;
	return output;
}