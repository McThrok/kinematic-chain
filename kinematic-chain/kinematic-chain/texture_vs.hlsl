cbuffer perObjectBuffer : register(b0)
{
	float4x4 wvpMatrix;
	float4x4 worldMatrix;
};

struct VS_INPUT
{
	float3 inPos : POSITION;
	float3 inTex : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 outPosition : SV_POSITION;
	float3 inTex : TEXCOORD;
	float3 outWorldPos : WORLD_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	float3 pos = input.inPos;

	output.outPosition = mul(wvpMatrix, float4(pos, 1.0f));
	output.inTex = input.inTex;
	output.outWorldPos = mul(worldMatrix, float4(pos, 1.0f));

	return output;
}