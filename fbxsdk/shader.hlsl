// <定数バッファ(CPU側からの値受け取り場)>
cbuffer global {
	matrix gWVP;    // <変換行列>
};

// <頂点シェーダ>
float4 VS(float4 Pos : POSITION) : SV_POSITION{
	return mul(Pos, gWVP);
}

// <ピクセルシェーダ>
float4 PS(float4 Pos : SV_POSITION) : SV_Target{
	return float4(0.5, 0.5, 1.0, 1.0);
}