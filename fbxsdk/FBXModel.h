#pragma once

// <FBX SDK ヘルパークラス>
// <作成日 : 2019 / 08 / 02>
// <作成者 : Nyarll>
// GitHub : https://github.com/Nyarll

// <FBXファイルから直接モデルを表示するためのSDK>
// <ダウンロードページ>
// https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2019-5
// <参考サイト>
// https://www.tkng45memo.com/fbxmesh
// <以下の場所に FBX SDK をインストールしましょう(デフォルトの場所)>
// C:\Program Files\Autodesk\FBX\FBX SDK\2019.5
// DirectXTKと同様に、追加のインクルードファイルなどの指定をしましょう(参考サイトを見ればおそらく解決します)

// <付属品(?)のシェーダーファイル[shader.hlsl]について>
// 名称変更(自己責任)
// Visual Studio から、 shader.hlsl のプロパティ -> HLSLコンパイラ -> 全般
// 全般タブ内の シェーダーの種類 : エフェクト(/fx) , シェーダーモデル : Shader Model 5.0(/5_0)
// に設定しましょう

// <include>
#include <DirectXMath.h>
#include <SimpleMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <fbxsdk.h>

// <link FBX SDK library>
// -mt(マルチスレッドデバッグ(MTd))
//#pragma comment(lib, "libfbxsdk-mt.lib")
//#pragma comment(lib, "zlib-mt.lib")
//#pragma comment(lib, "libxml2-mt.lib")
//#pragma comment(lib, "libfbxsdk.lib")
#pragma comment(lib, "d3dcompiler.lib")


namespace FBXSDK_Helper
{
	// <FBX Model class>
	class FBX_Model
	{
	protected:
		// <一つの頂点情報を格納する構造体>
		struct VERTEX {
			DirectX::XMFLOAT3 Pos;
		};

		// <GPU>
		struct CONSTANT_BUFFER {
			DirectX::XMMATRIX mWVP;
		};

	protected:
		ID3D11RasterizerState *pRasterizerState;
		ID3D11VertexShader *pVertexShader;
		ID3D11InputLayout *pVertexLayout;
		ID3D11PixelShader *pPixelShader;
		ID3D11Buffer *m_constantBuffer;

		FbxManager *m_fbxManager = nullptr;
		FbxScene *m_fbxScene = NULL;
		FbxNode* m_meshNode = NULL;
		FbxMesh *m_mesh = NULL;
		ID3D11Buffer *VerBuffer = NULL;
		ID3D11Buffer *IndBuffer = NULL;
		VERTEX *vertices;

		int AnimStackNumber = 0;
		FbxTime FrameTime, timeCount, start, stop;

	public:
		FBX_Model();
		~FBX_Model();

		// <描画>
		virtual void Draw(
			ID3D11DeviceContext1* context,
			DirectX::SimpleMath::Matrix& world,
			DirectX::SimpleMath::Matrix& view,
			DirectX::SimpleMath::Matrix& proj);

		// <モデル作成>
		virtual void Create(
			HWND hwnd,
			ID3D11Device1* device,
			ID3D11DeviceContext1* context,
			ID3D11RenderTargetView* renderTargetView,
			const char* fbxfile_path);

		// <破棄>
		void Destroy();

	protected:
		virtual void FBX_Import(const char* fbxfile_path);
		virtual void FBX_GetVertex();
		virtual void FBX_SetVertexData(ID3D11Device1* device);
	};

	class FBX_AnimationModel : public FBX_Model
	{
	private:
		FbxNode* m_meshNode = NULL;

		int AnimStackNumber = 0;
		FbxTime FrameTime, timeCount, start, stop;
	public:
		FBX_AnimationModel();
		~FBX_AnimationModel();

		// <描画>
		virtual void Draw(
			ID3D11DeviceContext1* context,
			DirectX::SimpleMath::Matrix& world,
			DirectX::SimpleMath::Matrix& view,
			DirectX::SimpleMath::Matrix& proj)override;

		// <モデル作成>
		virtual void Create(
			HWND hwnd,
			ID3D11Device1* device,
			ID3D11DeviceContext1* context,
			ID3D11RenderTargetView* renderTargetView,
			const char* fbxfile_path)override;

	protected:
		virtual void FBX_Import(const char* fbxfile_path)override;
		virtual void FBX_SetVertexData(ID3D11Device1* device)override;

	};
}