Texture2D txDiffuse : register( t0 );
SamplerState samLinear : register( s0 );

cbuffer cbMaterial : register( b0 )
{
	float4 ambient;
	float4 diffuse;
	float3 specular;
	float power;
	float4 emmisive;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};

float4 PS( PS_INPUT input) : SV_Target
{
	float4 red = { 1.0f, 0.0f, 0.0f, 1.0f };
	float4 black = { 0,0,0,1.0f };
	// テクスチャがないやつは表示されない
	//return txDiffuse.Sample( samLinear, input.Tex );
	
	//return input.Pos;	// オブジェクトは白くなる

	// オブジェクトは黒くなる
	//return ambient;
	//return diffuse;

	// マテリアルの色
	//return emmisive;

	return black;
	
	//return ambient + diffuse + emmisive;
}