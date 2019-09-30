#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#ifndef FBXSDK_NEW_API
#define FBXSDK_NEW_API	// <新しいバージョンを使うとき用>
#endif
#include <DirectXMath.h>
#include <SimpleMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <fbxsdk.h>
#include <Windows.h>
#include <map>

namespace FBX_LOADER
{
	// <一つの頂点情報を格納する構造体>
	struct VERTEX {
		DirectX::XMFLOAT3 Pos;
	};

	// <GPU(シェーダ側)へ送る数値をまとめた構造体>
	struct CONSTANT_BUFFER {
		DirectX::XMMATRIX mWVP;
	};

	class FbxModel
	{
	private:
		ID3D11RasterizerState *pRasterizerState;
		ID3D11VertexShader *pVertexShader;
		ID3D11InputLayout *pVertexLayout;
		ID3D11PixelShader *pPixelShader;
		ID3D11Buffer *m_constantBuffer;

		ID3D11Buffer *VerBuffer = NULL;
		ID3D11Buffer *IndBuffer = NULL;

		int AnimStackNumber = 0;
		FbxTime FrameTime, timeCount, start, stop;

		FbxManager* m_fbxManager;
		FbxImporter* m_fbxImporter;
		FbxScene* m_fbxScene;

		std::vector<std::string> m_fbxMeshNames;
		std::vector<FbxMesh*> m_fbxMeshes;

		ID3D11Device* m_device;
		ID3D11DeviceContext* m_context;
		ID3D11RenderTargetView* m_renderTargetView;
		// <ビューポートの設定>
		D3D11_VIEWPORT m_vp;

		bool is_animation = false;

		bool Draw(FbxNode* pNode,
			DirectX::SimpleMath::Matrix world, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj);

	public:
		FbxModel();
		~FbxModel();

		bool Load(HWND hwnd, ID3D11Device* device,
			ID3D11DeviceContext* context, ID3D11RenderTargetView* renderTargetView,
			const char* file_name, bool isAnimation = false);

		void Draw(ID3D11Device* device,
			ID3D11DeviceContext* context,
			DirectX::SimpleMath::Matrix world,
			DirectX::SimpleMath::Matrix view,
			DirectX::SimpleMath::Matrix proj);
	};
}