// <�萔�o�b�t�@(CPU������̒l�󂯎���)>
cbuffer global {
	matrix gWVP;    // <�ϊ��s��>
};

// <���_�V�F�[�_>
float4 VS(float4 Pos : POSITION) : SV_POSITION{
	return mul(Pos, gWVP);
}

// <�s�N�Z���V�F�[�_>
float4 PS(float4 Pos : SV_POSITION) : SV_Target{
	return float4(0.5, 0.5, 1.0, 1.0);
}