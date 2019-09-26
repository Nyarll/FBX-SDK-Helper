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
	// �e�N�X�`�����Ȃ���͕\������Ȃ�
	//return txDiffuse.Sample( samLinear, input.Tex );
	
	//return input.Pos;	// �I�u�W�F�N�g�͔����Ȃ�

	// �I�u�W�F�N�g�͍����Ȃ�
	//return ambient;
	//return diffuse;

	// �}�e���A���̐F
	//return emmisive;

	return black;
	
	//return ambient + diffuse + emmisive;
}