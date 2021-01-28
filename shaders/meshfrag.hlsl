struct VSOutput
{
	float4 pos : SV_POSITION;
	float3 col : COLOR0;
	float2 texCoord : TEXCOORD0;
};

Texture2D texture : register(t1);
SamplerState textureSampler : register(s1);

float3 main(VSOutput input) : SV_Target
{
	return texture.Sample(textureSampler, input.texCoord).xyz;
}