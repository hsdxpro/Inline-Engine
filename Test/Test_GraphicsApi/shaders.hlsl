
// Transform constants
struct Transform {
	float4x4 world;
	float4x4 viewProj;
};
ConstantBuffer<Transform> transform : register(b0);
// Misc constants
struct MiscInfo {
	float4 color;
};
ConstantBuffer<MiscInfo> misc : register(b8);

// Textures
Texture2D<float4> tex : register(t0);

// Samplers
sampler samp : register(s0);


struct VSInput {
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texcoord : TEX;
};

struct PSInput {
	float4 position : SV_POSITION;
	float3 normal : COLOR;
	float2 texcoord : TEX;
};

// Vertex shader
PSInput VSmain(VSInput input) {
	PSInput result;

	float4x4 wvp = mul(transform.viewProj, transform.world);
	result.position = mul(wvp, float4(input.position, 1));
	float3x3 worldRot = (float3x3)transform.world;
	result.normal = mul(worldRot, input.normal);
	result.texcoord = input.texcoord;

	return result;
}

// Pixel shader
float4 PSmain(PSInput input) : SV_TARGET {
	float ndotl = saturate(dot(input.normal, normalize(float3(-0.5, 0.5, 0.5))));
	//float4 color = float3(0.8, 0.68, 0.6);
	float3 color = tex.Sample(samp, input.texcoord).rgb;
	return float4(color*(ndotl+float3(0.12, 0.14, 0.18)), 1);
}
