
struct UniformBufferObject
{
	float4x4 model;
	float4x4 view;
	float4x4 proj;
	float3 lightDir;
} ubo;

//cbuffer ubo : register(b0) { UniformBufferObject ubo; }

struct VSInput
{
	float3 pos : POSITION0; //[[vk::location(0)]]
	float3 normal : NORMAL0;
	float2 texCoord : TEXCOORD0;
	float3 tangent : TANGENT0;
	float3 bitangent : BITANGENT0;
};

struct VSOutput
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 texCoord : TEXCOORD0;
	float3 tangent : TANGENT0;
	float3 bitangent : BITANGENT0;
};

VSOutput main(VSInput input)
{
	VSOutput output;
	output.pos = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.pos, 1.0f))));
	output.normal = normalize(mul((float3x3)ubo.model, input.normal));
	output.tangent = normalize(mul((float3x3)ubo.model, input.tangent));
	output.bitangent = normalize(mul((float3x3)ubo.model, input.bitangent));
	output.texCoord = input.texCoord;
	return output;
}