struct VSOutput
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float2 texCoord : TEXCOORD0;
	float3 tangent : TANGENT0;
	float3 bitangent : BITANGENT0;
};

struct UniformBufferObject
{
	float4x4 model;
	float4x4 view;
	float4x4 proj;
	float3 lightDir;
} ubo;

Texture2D textureMap : register(t1);
SamplerState textureSampler : register(s1);
Texture2D normalMap : register(t2);
SamplerState normalSampler : register(s2);

float3 main(VSOutput input) : SV_Target
{
	float3 surfaceColor = float3(1,1,1);//textureMap.Sample(textureSampler, input.texCoord).xyz;
	float3 coolColor = float3(0,0,0); //float3(0, 0, 0.55) + 0.25*surfaceColor;
	float3 warmColor = surfaceColor; //float3(0.3, 0.3, 0) + 0.25*surfaceColor;
	float3 specColor = float3(1, 1, 1);
	
	float3 normal = normalMap.Sample(normalSampler, input.texCoord).xyz*2.0 - 1.0;
	//normal = normalize(mul(float3x3(input.tangent, input.bitangent, input.normal), float3(0,0,1)));
	float3 lightDir = normalize(ubo.lightDir);
	float t = (dot(normal, lightDir) + 1)/2;
	float3 r = 2*dot(normal, lightDir)*normal - lightDir;
	float3 view = float3(2,2,2) - input.pos;
	float s = saturate(100*dot(r, view) - 97);
	//return s*specColor + (1-s)*(t*warmColor + (1-t)*coolColor);
	return (t*warmColor + (1-t)*coolColor);
	return (normal + 1)/2;
}