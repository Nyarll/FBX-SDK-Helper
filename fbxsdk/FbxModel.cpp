
#include "FbxModel.h"

FBX_LOADER::FbxModel::FbxModel()
{
}

FBX_LOADER::FbxModel::~FbxModel()
{
	m_fbxImporter->Destroy();
	m_fbxManager->Destroy();
}

bool FBX_LOADER::FbxModel::Load(HWND hwnd, ID3D11Device * device, ID3D11DeviceContext * context, ID3D11RenderTargetView* renderTargetView, const char * file_name)
{
	// <nullptr チェック>
	if (!device)return false;
	if (!context)return false;
	if (!renderTargetView)return false;
	if (!file_name)return false;

	m_device = device;
	m_context = context;
	m_renderTargetView = renderTargetView;

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

	m_vp = vp;

	// <シェーダの設定>
	ID3DBlob *pCompileVS = NULL;
	ID3DBlob *pCompilePS = NULL;
	D3DCompileFromFile(L"shader.hlsl", NULL, NULL, "VS", "vs_5_0", NULL, 0, &pCompileVS, NULL);
	assert(pCompileVS && "pCompileVS is nullptr !");
	(device->CreateVertexShader(pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), NULL, &pVertexShader));
	D3DCompileFromFile(L"shader.hlsl", NULL, NULL, "PS", "ps_5_0", NULL, 0, &pCompilePS, NULL);
	assert(pCompileVS && "pCompilePS is nullptr !");
	(device->CreatePixelShader(pCompilePS->GetBufferPointer(), pCompilePS->GetBufferSize(), NULL, &pPixelShader));

	// <頂点レイアウト>
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	(device->CreateInputLayout(layout, 1, pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), &pVertexLayout));
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
	(device->CreateBuffer(&cb, NULL, &m_constantBuffer));

	// <FBX Manager作成>
	m_fbxManager = FbxManager::Create();
	if (!m_fbxManager)return false;

	// <FBX Importer作成>
	m_fbxImporter = FbxImporter::Create(m_fbxManager, "");
	if (!m_fbxImporter)return false;

	// <初期化・読み込み>
	m_fbxImporter->Initialize(file_name);
	m_fbxScene = FbxScene::Create(m_fbxManager, "model");
	m_fbxImporter->Import(m_fbxScene);

	FbxArray<FbxString*> AnimStackNameArray;
	m_fbxScene->FillAnimStackNameArray(AnimStackNameArray);
	FbxAnimStack *AnimationStack = m_fbxScene->FindMember<FbxAnimStack>(AnimStackNameArray[AnimStackNumber]->Buffer());
	m_fbxScene->SetCurrentAnimationStack(AnimationStack);

	FbxTakeInfo *takeInfo = m_fbxScene->GetTakeInfo(*(AnimStackNameArray[AnimStackNumber]));
	start = takeInfo->mLocalTimeSpan.GetStart();
	stop = takeInfo->mLocalTimeSpan.GetStop();
	FrameTime.SetTime(0, 0, 0, 1, 0, m_fbxScene->GetGlobalSettings().GetTimeMode());
	timeCount = start;

	int meshCount = m_fbxScene->GetSrcObjectCount<FbxMesh>();
	for (int i = 0; i < meshCount; ++i)
	{
		// <たったこれだけで全てのメッシュデータを取得できる>
		FbxMesh* mesh = m_fbxScene->GetSrcObject<FbxMesh>(i);
		std::string name = mesh->GetName();
		m_fbxMeshNames.push_back(name);
		//m_fbxMeshes.insert({ mesh, name });
		m_fbxMeshes.push_back(mesh);
	}

	// <ラスタライザの設定>
	D3D11_RASTERIZER_DESC rdc = {};
	rdc.CullMode = D3D11_CULL_BACK;
	rdc.FillMode = D3D11_FILL_SOLID;
	rdc.FrontCounterClockwise = TRUE;
	(device->CreateRasterizerState(&rdc, &pRasterizerState));


	return true;
}

void FBX_LOADER::FbxModel::Draw(ID3D11Device * device, ID3D11DeviceContext * context,
	DirectX::SimpleMath::Matrix world, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj)
{
	// ----- Animation -----
	timeCount += FrameTime;
	if (timeCount > stop) timeCount = start;

	this->Draw(m_fbxScene->GetRootNode(), timeCount, world, view, proj);
}

bool FBX_LOADER::FbxModel::Draw(FbxNode * pNode, FbxTime & time,
	DirectX::SimpleMath::Matrix world, DirectX::SimpleMath::Matrix view, DirectX::SimpleMath::Matrix proj)
{
	// 移動、回転、拡大のための行列を作成
	FbxMatrix globalPosition = pNode->EvaluateGlobalTransform(timeCount);
	FbxVector4 t0 = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	FbxVector4 r0 = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
	FbxVector4 s0 = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
	FbxAMatrix geometryOffset = FbxAMatrix(t0, r0, s0);

	int Count = 1;// m_fbxMeshes.size();
	for (int i = 0; i < Count; i++)
	{
		FbxMesh* mesh = m_fbxMeshes[i];
		// 各頂点に掛けるための最終的な行列の配列
		FbxMatrix *clusterDeformation = new FbxMatrix[mesh->GetControlPointsCount()];
		memset(clusterDeformation, 0, sizeof(FbxMatrix) * mesh->GetControlPointsCount());

		FbxSkin *skinDeformer = (FbxSkin *)mesh->GetDeformer(0, FbxDeformer::eSkin);
		int clusterCount = skinDeformer->GetClusterCount();
		// 各クラスタから各頂点に影響を与えるための行列作成
		for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
			// クラスタ(ボーン)の取り出し
			FbxCluster *cluster = skinDeformer->GetCluster(clusterIndex);
			FbxMatrix vertexTransformMatrix;
			FbxAMatrix referenceGlobalInitPosition;
			FbxAMatrix clusterGlobalInitPosition;
			FbxMatrix clusterGlobalCurrentPosition;
			FbxMatrix clusterRelativeInitPosition;
			FbxMatrix clusterRelativeCurrentPositionInverse;
			cluster->GetTransformMatrix(referenceGlobalInitPosition);
			referenceGlobalInitPosition *= geometryOffset;
			cluster->GetTransformLinkMatrix(clusterGlobalInitPosition);
			clusterGlobalCurrentPosition = cluster->GetLink()->EvaluateGlobalTransform(timeCount);
			clusterRelativeInitPosition = clusterGlobalInitPosition.Inverse() * referenceGlobalInitPosition;
			clusterRelativeCurrentPositionInverse = globalPosition.Inverse() * clusterGlobalCurrentPosition;
			vertexTransformMatrix = clusterRelativeCurrentPositionInverse * clusterRelativeInitPosition;
			// 上で作った行列に各頂点毎の影響度(重み)を掛けてそれぞれに加算
			for (int i = 0; i < cluster->GetControlPointIndicesCount(); i++) {
				int index = cluster->GetControlPointIndices()[i];
				double weight = cluster->GetControlPointWeights()[i];
				FbxMatrix influence = vertexTransformMatrix * weight;
				clusterDeformation[index] += influence;
			}
		}

		VERTEX* vertices = new VERTEX[mesh->GetControlPointsCount()];
		// 最終的な頂点座標を計算しVERTEXに変換
		for (int i = 0; i < mesh->GetControlPointsCount(); i++) {
			FbxVector4 outVertex = clusterDeformation[i].MultNormalize(mesh->GetControlPointAt(i));
			vertices[i].Pos.x = (FLOAT)outVertex[0];
			vertices[i].Pos.y = (FLOAT)outVertex[1];
			vertices[i].Pos.z = (FLOAT)outVertex[2];
		}

		delete[] clusterDeformation;
		// ---------------------

		D3D11_BUFFER_DESC bd_vertex;
		bd_vertex.ByteWidth = sizeof(VERTEX) * mesh->GetControlPointsCount();
		bd_vertex.Usage = D3D11_USAGE_DYNAMIC;
		bd_vertex.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd_vertex.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd_vertex.MiscFlags = 0;
		bd_vertex.StructureByteStride = 0;
		if (FAILED(m_device->CreateBuffer(&bd_vertex, NULL, &VerBuffer))) {
			assert(false && "Missing !");
		}

		// インデックスデータの取り出しとバッファの設定
		D3D11_BUFFER_DESC bd_index;
		bd_index.ByteWidth = sizeof(int) * mesh->GetPolygonVertexCount();
		bd_index.Usage = D3D11_USAGE_DEFAULT;
		bd_index.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd_index.CPUAccessFlags = 0;
		bd_index.MiscFlags = 0;
		bd_index.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA data_index;
		data_index.pSysMem = mesh->GetPolygonVertices();
		if (FAILED(m_device->CreateBuffer(&bd_index, &data_index, &IndBuffer)))
		{
			assert(false && "Missing !");
		}

		D3D11_MAPPED_SUBRESOURCE pdata;
		CONSTANT_BUFFER cb;
		// パラメータの受け渡し(定数)
		cb.mWVP = DirectX::XMMatrixTranspose(world * view * proj);
		m_context->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pdata);
		memcpy_s(pdata.pData, pdata.RowPitch, (void*)(&cb), sizeof(cb));
		m_context->Unmap(m_constantBuffer, 0);

		// パラメータの受け渡し(頂点)
		m_context->Map(VerBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pdata);
		memcpy_s(pdata.pData, pdata.RowPitch, (void*)(vertices), sizeof(VERTEX) * mesh->GetControlPointsCount());
		m_context->Unmap(VerBuffer, 0);

		UINT stride = sizeof(VERTEX);
		UINT offset = 0;
		m_context->IASetVertexBuffers(0, 1, &VerBuffer, &stride, &offset);
		m_context->IASetIndexBuffer(IndBuffer, DXGI_FORMAT_R32_UINT, 0);
		m_context->IASetInputLayout(pVertexLayout);
		m_context->VSSetConstantBuffers(0, 1, &m_constantBuffer);
		m_context->PSSetConstantBuffers(0, 1, &m_constantBuffer);
		m_context->VSSetShader(pVertexShader, NULL, 0);
		m_context->PSSetShader(pPixelShader, NULL, 0);
		m_context->RSSetState(pRasterizerState);
		m_context->OMSetRenderTargets(1, &m_renderTargetView, NULL);
		m_context->RSSetViewports(1, &m_vp);

		// 2019 : 09 : 30 <ビルドは通るが、ここでエラーを吐く>
		// <描画実行>
		m_context->DrawIndexed(mesh->GetControlPointsCount(), 0, 0);

		delete[] vertices;
		VerBuffer->Release();
		VerBuffer = nullptr;
		IndBuffer->Release();
		IndBuffer = nullptr;
	}
	return true;
}
