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

// <include>
#include <DirectXMath.h>
#include <SimpleMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <fbxsdk.h>

#include "Resource.h"
#include "CFBXRendererDX.h"

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
	private:
		std::unique_ptr<FBX_LOADER::CFBXRenderDX11>	m_model;
		
		ID3D11VertexShader* m_vertexShader = nullptr;
		ID3D11PixelShader* m_pixelShader = nullptr;
		ID3D11Buffer* m_pcBuffer = nullptr;
		ID3D11RasterizerState* m_pRS = nullptr;
		ID3D11BlendState* m_blendState = nullptr;

		ID3D11Buffer* m_transformStructuredBuffer = nullptr;
		ID3D11ShaderResourceView* m_transformSRV = nullptr;

	protected:

		struct CBFBXMATRIX
		{
			DirectX::XMMATRIX mWorld;
			DirectX::XMMATRIX mView;
			DirectX::XMMATRIX mProj;
			DirectX::XMMATRIX mWVP;
		};
		struct SRVPerInstanceData
		{
			DirectX::XMMATRIX mWorld;
		};

		HRESULT InitApp(ID3D11Device* device, ID3D11DeviceContext* context, const char* file_name);
		void CleanupApp();
		HRESULT SetupTransformSRV(ID3D11Device* device);
		void	SetMatrix(ID3D11DeviceContext* context);

	public:
		FBX_Model() {};
		~FBX_Model() 
		{
		};

		void Create(ID3D11Device* device, ID3D11DeviceContext* context, const char* fileName);

		void Release();

		void Render(ID3D11DeviceContext* context, 
			ID3D11DepthStencilState* stencil_state,
			const DirectX::SimpleMath::Matrix& world,
			const DirectX::SimpleMath::Matrix& view, 
			const DirectX::SimpleMath::Matrix& proj);
	};

	
}