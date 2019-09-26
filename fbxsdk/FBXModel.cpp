#include "FBXModel.h"

HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DCompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		if (pErrorBlob) pErrorBlob->Release();
		return hr;
	}
	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

HRESULT FBXSDK_Helper::FBX_Model::InitApp(
	ID3D11Device* device,
	ID3D11DeviceContext* context,
	const char* file_name)
{
	HRESULT hr = S_OK;

	m_model = std::make_unique<FBX_LOADER::CFBXRenderDX11>();
	hr = m_model->LoadFBX(file_name, device);

	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"FBX Error", L"Error", MB_OK);
		return hr;
	}

	// Compile the vertex shader
	ID3DBlob* pVSBlob = NULL;
	hr = CompileShaderFromFile(L"simpleRenderVS.hlsl", "vs_main", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// Todo: InputLayoutの作成には頂点シェーダが必要なのでこんなタイミングでCreateするのをなんとかしたい

	hr = m_model->CreateInputLayout(device, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), layout, numElements);

	pVSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Compile the pixel shader
	ID3DBlob* pPSBlob = NULL;
	hr = CompileShaderFromFile(L"simpleRenderPS.hlsl", "PS", "ps_4_0", &pPSBlob);
	if (FAILED(hr))
	{
		MessageBox(NULL,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), NULL, &m_pixelShader);
	pPSBlob->Release();
	if (FAILED(hr))
		return hr;

	// Create Constant Buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(CBFBXMATRIX);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = device->CreateBuffer(&bd, NULL, &m_pcBuffer);
	if (FAILED(hr))
		return hr;

	//
	D3D11_RASTERIZER_DESC rsDesc;
	ZeroMemory(&rsDesc, sizeof(D3D11_RASTERIZER_DESC));
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.FrontCounterClockwise = false;
	rsDesc.DepthClipEnable = FALSE;
	device->CreateRasterizerState(&rsDesc, &m_pRS);
	context->RSSetState(m_pRS);

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;      ///tryed D3D11_BLEND_ONE ... (and others desperate combinations ... )
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;     ///tryed D3D11_BLEND_ONE ... (and others desperate combinations ... )
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	device->CreateBlendState(&blendDesc, &m_blendState);

	// SpriteBatch
	//g_pSpriteBatch = new DirectX::SpriteBatch(g_pImmediateContext);
	// SpriteFont
	//g_pFont = new DirectX::SpriteFont(g_pd3dDevice, L"Assets\\Arial.spritefont");

	return hr;
}

void FBXSDK_Helper::FBX_Model::CleanupApp()
{
	if (m_transformSRV)
	{
		delete m_transformSRV;
		m_transformSRV = nullptr;
	}
	if (m_transformStructuredBuffer)
	{
		delete m_transformStructuredBuffer;
		m_transformStructuredBuffer = nullptr;
	}
	if (m_blendState)
	{
		delete m_blendState;
		m_blendState = nullptr;
	}
	m_model->Release();
	m_model.reset();

	if (m_pcBuffer)
	{
		delete m_pcBuffer;
		m_pcBuffer = nullptr;
	}
	if (m_pRS)
	{
		delete m_pRS;
		m_pRS = nullptr;
	}
	if (m_vertexShader)
	{
		delete m_vertexShader;
		m_vertexShader = nullptr;
	}
	if (m_pixelShader)
	{
		delete m_pixelShader;
		m_pixelShader = nullptr;
	}
}

HRESULT FBXSDK_Helper::FBX_Model::SetupTransformSRV(ID3D11Device* device)
{
	HRESULT hr = S_OK;

	const uint32_t count = 32;
	const uint32_t stride = static_cast<uint32_t>(sizeof(SRVPerInstanceData));

	// Create StructuredBuffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = stride * count;
	bd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bd.StructureByteStride = stride;
	hr = device->CreateBuffer(&bd, NULL, &m_transformStructuredBuffer);
	if (FAILED(hr))
		return hr;

	// Create ShaderResourceView
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;   // 拡張されたバッファーであることを指定する
	srvDesc.BufferEx.FirstElement = 0;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.BufferEx.NumElements = count;                  // リソース内の要素の数

														   // 構造化バッファーをもとにシェーダーリソースビューを作成する
	hr = device->CreateShaderResourceView(m_transformStructuredBuffer, &srvDesc, &m_transformSRV);
	if (FAILED(hr))
		return hr;

	return hr;
}

void FBXSDK_Helper::FBX_Model::SetMatrix(ID3D11DeviceContext* context)
{
	HRESULT hr = S_OK;
	const uint32_t count = 32U;
	const float offset = -(32U * 60.0f / 2.0f);
	DirectX::XMMATRIX mat;

	D3D11_MAPPED_SUBRESOURCE MappedResource;
	hr = context->Map(m_transformStructuredBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);

	SRVPerInstanceData*	pSrvInstanceData = (SRVPerInstanceData*)MappedResource.pData;

	for (uint32_t i = 0; i < count; i++)
	{
		mat = DirectX::XMMatrixTranslation(0, 0, i*60.0f + offset);
		pSrvInstanceData[i].mWorld = (mat);
	}

	context->Unmap(m_transformStructuredBuffer, 0);
}

void FBXSDK_Helper::FBX_Model::Create(ID3D11Device * device, ID3D11DeviceContext * context, const char * fileName)
{
	if (FAILED(InitApp(device, context, fileName))) {}
	if (FAILED(this->SetupTransformSRV(device))) {}
}

void FBXSDK_Helper::FBX_Model::Release()
{
	this->CleanupApp();
}

void FBXSDK_Helper::FBX_Model::Render(ID3D11DeviceContext* context,
	ID3D11DepthStencilState* stencil_state,
	const DirectX::SimpleMath::Matrix& world,
	const DirectX::SimpleMath::Matrix& view,
	const DirectX::SimpleMath::Matrix& proj)
{
	float blendFactors[4] = { D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO };
	context->RSSetState(m_pRS);
	context->OMSetBlendState(m_blendState, blendFactors, 0xffffffff);
	context->OMSetDepthStencilState(stencil_state, 0);

	// モデルを描画

	// FBX Modelのnode数を取得
	size_t nodeCount = m_model->GetNodeCount();

	context->VSSetShader(m_vertexShader, NULL, 0);
	context->VSSetConstantBuffers(0, 1, &m_pcBuffer);
	context->PSSetShader(m_pixelShader, NULL, 0);

	// 全ノードを描画
	for (size_t j = 0; j < nodeCount; j++)
	{

		DirectX::XMMATRIX mLocal;
		m_model->GetNodeMatrix(j, &mLocal.r[0].m128_f32[0]);	// このnodeのMatrix
		DirectX::SimpleMath::Matrix local(mLocal);

		D3D11_MAPPED_SUBRESOURCE MappedResource;
		context->Map(m_pcBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);

		CBFBXMATRIX*	cbFBX = (CBFBXMATRIX*)MappedResource.pData;

		// 左手系
		cbFBX->mWorld = (world);
		cbFBX->mView = (view);
		cbFBX->mProj = (proj);

		cbFBX->mWVP = DirectX::XMMatrixTranspose(local * world * view * proj);

		context->Unmap(m_pcBuffer, 0);

		SetMatrix(context);

		auto mate = m_model->GetNodeMaterial(j);

		for (auto& material : mate)
		{
			if (material.pMaterialCb)
				context->UpdateSubresource(material.pMaterialCb, 0, NULL, &material.materialConstantData, 0, 0);

			context->VSSetShaderResources(0, 1, &m_transformSRV);
			context->PSSetShaderResources(0, 1, &material.pSRV);
			context->PSSetConstantBuffers(0, 1, &material.pMaterialCb);
			context->PSSetSamplers(0, 1, &material.pSampler);

			m_model->RenderNode(context, j);
		}
	}
}
