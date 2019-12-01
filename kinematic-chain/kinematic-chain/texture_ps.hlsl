cbuffer perObjectBuffer : register(b0)
{
	float4x4 wvpMatrix;
	float4x4 worldMatrix;
};

struct PS_INPUT
{
	float4 outPosition : SV_POSITION;
	float3 inTex : TEXCOORD;
	float3 outWorldPos : WORLD_POSITION;
};

Texture2D objTexture : TEXTURE: register(t0);
SamplerState objSamplerState : SAMPLER: register(s0);

float4 main(PS_INPUT input) : SV_TARGET
{
	float4 sampleColor = objTexture.Sample(objSamplerState, input.inTex);

	return float4(sampleColor.rgb, 1);
}