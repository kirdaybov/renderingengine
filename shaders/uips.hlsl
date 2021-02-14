Texture2D fontTexture : register(t1);
SamplerState fontSampler : register(s1);

struct VertexOutput
{
	float2 uv : TEXCOORD0;
	float4 color : COLOR0;
};

float4 main(VertexOutput input) : SV_TARGET
{
	return input.color * fontTexture.Sample(fontSampler, input.uv);
}