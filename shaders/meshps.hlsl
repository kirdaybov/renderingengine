struct VSOutput
{
	float4 pos : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

Texture2D textureMap : register(t1);
SamplerState textureSampler : register(s1);

float3 main(VSOutput input) : SV_Target
{
	return textureMap.Sample(textureSampler, input.texCoord).xyz;
}