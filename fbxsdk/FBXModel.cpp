#include "pch.h"
#include "FBXModel.h"

FBXSDK_Helper::FBX_Model::FBX_Model()
{
}

FBXSDK_Helper::FBX_Model::~FBX_Model()
{
	Destroy();
}

void FBXSDK_Helper::FBX_Model::Draw(ID3D11DeviceContext1* context, DirectX::SimpleMath::Matrix world,
	DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj)
{
	// パラメータの受け渡し
	D3D11_MAPPED_SUBRESOURCE pdata;
	CONSTANT_BUFFER cb;
	cb.mWVP = DirectX::XMMatrixTranspose(world * view * proj);
	context->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pdata);
	memcpy_s(pdata.pData, pdata.RowPitch, (void*)(&cb), sizeof(cb));
	context->Unmap(m_constantBuffer, 0);

	// 描画実行
	context->DrawIndexed(m_mesh->GetPolygonVertexCount(), 0, 0);
}

void FBXSDK_Helper::FBX_Model::Create(
	HWND hwnd,
	ID3D11Device1* device,
	ID3D11DeviceContext1* context,
	ID3D11RenderTargetView* renderTargetView,
	const char* fbxfile_path)
{
	// <パイプライン準備>
	RECT csize;
	GetClientRect(hwnd, &csize);
	int CWIDTH = csize.right;
	int CHEIGHT = csize.bottom;

	// <ビューポートの設定>
	D3D11_VIEWPORT vp;
	vp.Width = CWIDTH;
	vp.Height = CHEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	// <シェーダの設定>
	ID3DBlob *pCompileVS = NULL;
	ID3DBlob *pCompilePS = NULL;
	D3DCompileFromFile(L"shader.hlsl", NULL, NULL, "VS", "vs_5_0", NULL, 0, &pCompileVS, NULL);
	assert(pCompileVS && "pCompileVS is nullptr !");
	device->CreateVertexShader(pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), NULL, &pVertexShader);
	D3DCompileFromFile(L"shader.hlsl", NULL, NULL, "PS", "ps_5_0", NULL, 0, &pCompilePS, NULL);
	assert(pCompileVS && "pCompilePS is nullptr !");
	device->CreatePixelShader(pCompilePS->GetBufferPointer(), pCompilePS->GetBufferSize(), NULL, &pPixelShader);

	// <頂点レイアウト>
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	device->CreateInputLayout(layout, 1, pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), &pVertexLayout);
	pCompileVS->Release();
	pCompilePS->Release();

	// <定数バッファの設定>
	D3D11_BUFFER_DESC cb;
	cb.ByteWidth = sizeof(CONSTANT_BUFFER);
	cb.Usage = D3D11_USAGE_DYNAMIC;
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.StructureByteStride = 0;
	device->CreateBuffer(&cb, NULL, &m_constantBuffer);

	// <FBX読み込み>
	FBX_Import(fbxfile_path);
	// <頂点データの取り出し>
	FBX_GetVertex();
	// <頂点データ用バッファの設定>
	FBX_SetVertexData(device);

	// <ラスタライザの設定>
	D3D11_RASTERIZER_DESC rdc = {};
	rdc.CullMode = D3D11_CULL_BACK;
	rdc.FillMode = D3D11_FILL_SOLID;
	rdc.FrontCounterClockwise = TRUE;
	device->CreateRasterizerState(&rdc, &pRasterizerState);

	// <オブジェクトの反映>
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &VerBuffer, &stride, &offset);
	context->IASetIndexBuffer(IndBuffer, DXGI_FORMAT_R32_UINT, 0);
	context->IASetInputLayout(pVertexLayout);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->VSSetConstantBuffers(0, 1, &m_constantBuffer);
	context->PSSetConstantBuffers(0, 1, &m_constantBuffer);
	context->VSSetShader(pVertexShader, NULL, 0);
	context->PSSetShader(pPixelShader, NULL, 0);
	context->RSSetState(pRasterizerState);
	context->OMSetRenderTargets(1, &renderTargetView, NULL);
	context->RSSetViewports(1, &vp);
}

void FBXSDK_Helper::FBX_Model::Destroy()
{
	pRasterizerState->Release();
	pVertexShader->Release();
	pVertexLayout->Release();
	pPixelShader->Release();
	m_constantBuffer->Release();

	m_fbxScene->Destroy();
	m_fbxManager->Destroy();
	VerBuffer->Release();
	if (vertices)
	{
		delete[] vertices;
	}
}

void FBXSDK_Helper::FBX_Model::FBX_Import(const char* fbxfile_path)
{
	// <FBX読み込み>
	m_fbxManager = FbxManager::Create();
	m_fbxScene = FbxScene::Create(m_fbxManager, "fbxscene");
	FbxString FileName(fbxfile_path);
	FbxImporter *fbxImporter = FbxImporter::Create(m_fbxManager, "imp");
	fbxImporter->Initialize(FileName.Buffer(), -1, m_fbxManager->GetIOSettings());
	fbxImporter->Import(m_fbxScene);
	fbxImporter->Destroy();
}

void FBXSDK_Helper::FBX_Model::FBX_GetVertex()
{
	// <頂点データの取り出し>
	for (int i = 0; i < m_fbxScene->GetRootNode()->GetChildCount(); i++) {
		if (m_fbxScene->GetRootNode()->GetChild(i)->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh) {
			m_mesh = m_fbxScene->GetRootNode()->GetChild(i)->GetMesh();
			break;
		}
	}
	vertices = new VERTEX[m_mesh->GetControlPointsCount()];
	for (int i = 0; i < m_mesh->GetControlPointsCount(); i++) {
		vertices[i].Pos.x = (FLOAT)m_mesh->GetControlPointAt(i)[0];
		vertices[i].Pos.y = (FLOAT)m_mesh->GetControlPointAt(i)[1];
		vertices[i].Pos.z = (FLOAT)m_mesh->GetControlPointAt(i)[2];
	}
}

void FBXSDK_Helper::FBX_Model::FBX_SetVertexData(ID3D11Device1* device)
{
	{
		D3D11_BUFFER_DESC bd_vertex;
		bd_vertex.ByteWidth = sizeof(VERTEX) * m_mesh->GetControlPointsCount();
		bd_vertex.Usage = D3D11_USAGE_DEFAULT;
		bd_vertex.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd_vertex.CPUAccessFlags = 0;
		bd_vertex.MiscFlags = 0;
		bd_vertex.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA data_vertex;
		data_vertex.pSysMem = vertices;
		device->CreateBuffer(&bd_vertex, &data_vertex, &VerBuffer);

		// インデックスデータの取り出しとバッファの設定
		D3D11_BUFFER_DESC bd_index;
		bd_index.ByteWidth = sizeof(int) * m_mesh->GetPolygonVertexCount();
		bd_index.Usage = D3D11_USAGE_DEFAULT;
		bd_index.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd_index.CPUAccessFlags = 0;
		bd_index.MiscFlags = 0;
		bd_index.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA data_index;
		data_index.pSysMem = m_mesh->GetPolygonVertices();
		device->CreateBuffer(&bd_index, &data_index, &IndBuffer);
	}
}

