Texture2D fontTexture : register(t0);
SamplerState fontSampler : register(s0);

struct VertexOutput
{
	float2 uv : TEXCOORD0;
	float4 color : COLOR0;
};

float4 main(VertexOutput input) : SV_TARGET
{
	return input.color * fontTexture.Sample(fontSampler, input.uv);
}