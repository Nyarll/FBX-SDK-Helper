
#include "FBXModel.h"

FBXSDK_Helper::FBX_Model::FBX_Model()
{
}

FBXSDK_Helper::FBX_Model::~FBX_Model()
{
	Destroy();
}

void FBXSDK_Helper::FBX_Model::Draw(ID3D11DeviceContext* context, DirectX::XMMATRIX& world,
	DirectX::XMMATRIX& view, DirectX::XMMATRIX& proj)
{
	// ----- Animation -----
	timeCount += FrameTime;
	if (timeCount > stop) timeCount = start;

	// �ړ��A��]�A�g��̂��߂̍s����쐬
	FbxMatrix globalPosition = m_meshNode->EvaluateGlobalTransform(timeCount);
	FbxVector4 t0 = m_meshNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	FbxVector4 r0 = m_meshNode->GetGeometricRotation(FbxNode::eSourcePivot);
	FbxVector4 s0 = m_meshNode->GetGeometricScaling(FbxNode::eSourcePivot);
	FbxAMatrix geometryOffset = FbxAMatrix(t0, r0, s0);

	// �e���_�Ɋ|���邽�߂̍ŏI�I�ȍs��̔z��
	FbxMatrix *clusterDeformation = new FbxMatrix[m_mesh->GetControlPointsCount()];
	memset(clusterDeformation, 0, sizeof(FbxMatrix) * m_mesh->GetControlPointsCount());

	FbxSkin *skinDeformer = (FbxSkin *)m_mesh->GetDeformer(0, FbxDeformer::eSkin);
	int clusterCount = skinDeformer->GetClusterCount();
	// �e�N���X�^����e���_�ɉe����^���邽�߂̍s��쐬
	for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
		// �N���X�^(�{�[��)�̎��o��
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
		// ��ō�����s��Ɋe���_���̉e���x(�d��)���|���Ă��ꂼ��ɉ��Z
		for (int i = 0; i < cluster->GetControlPointIndicesCount(); i++) {
			int index = cluster->GetControlPointIndices()[i];
			double weight = cluster->GetControlPointWeights()[i];
			FbxMatrix influence = vertexTransformMatrix * weight;
			clusterDeformation[index] += influence;
		}
	}

	// �ŏI�I�Ȓ��_���W���v�Z��VERTEX�ɕϊ�
	for (int i = 0; i < m_mesh->GetControlPointsCount(); i++) {
		FbxVector4 outVertex = clusterDeformation[i].MultNormalize(m_mesh->GetControlPointAt(i));
		vertices[i].Pos.x = (FLOAT)outVertex[0];
		vertices[i].Pos.y = (FLOAT)outVertex[1];
		vertices[i].Pos.z = (FLOAT)outVertex[2];
	}

	delete[] clusterDeformation;
	// ---------------------


	D3D11_MAPPED_SUBRESOURCE pdata;
	CONSTANT_BUFFER cb;
	// �p�����[�^�̎󂯓n��(�萔)
	cb.mWVP = DirectX::XMMatrixTranspose(world * view * proj);
	context->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pdata);
	memcpy_s(pdata.pData, pdata.RowPitch, (void*)(&cb), sizeof(cb));
	context->Unmap(m_constantBuffer, 0);

	// �p�����[�^�̎󂯓n��(���_)
	context->Map(VerBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pdata);
	memcpy_s(pdata.pData, pdata.RowPitch, (void*)(vertices), sizeof(VERTEX) * m_mesh->GetControlPointsCount());
	context->Unmap(VerBuffer, 0);

	// �`����s
	context->DrawIndexed(m_mesh->GetPolygonVertexCount(), 0, 0);
}

void FBXSDK_Helper::FBX_Model::Draw(ID3D11DeviceContext * context, DirectX::SimpleMath::Matrix & world, DirectX::SimpleMath::Matrix & view, DirectX::SimpleMath::Matrix & proj)
{
	// ----- Animation -----
	timeCount += FrameTime;
	if (timeCount > stop) timeCount = start;

	// �ړ��A��]�A�g��̂��߂̍s����쐬
	FbxMatrix globalPosition = m_meshNode->EvaluateGlobalTransform(timeCount);
	FbxVector4 t0 = m_meshNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	FbxVector4 r0 = m_meshNode->GetGeometricRotation(FbxNode::eSourcePivot);
	FbxVector4 s0 = m_meshNode->GetGeometricScaling(FbxNode::eSourcePivot);
	FbxAMatrix geometryOffset = FbxAMatrix(t0, r0, s0);

	// �e���_�Ɋ|���邽�߂̍ŏI�I�ȍs��̔z��
	FbxMatrix *clusterDeformation = new FbxMatrix[m_mesh->GetControlPointsCount()];
	memset(clusterDeformation, 0, sizeof(FbxMatrix) * m_mesh->GetControlPointsCount());

	FbxSkin *skinDeformer = (FbxSkin *)m_mesh->GetDeformer(0, FbxDeformer::eSkin);
	int clusterCount = skinDeformer->GetClusterCount();
	// �e�N���X�^����e���_�ɉe����^���邽�߂̍s��쐬
	for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++) {
		// �N���X�^(�{�[��)�̎��o��
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
		// ��ō�����s��Ɋe���_���̉e���x(�d��)���|���Ă��ꂼ��ɉ��Z
		for (int i = 0; i < cluster->GetControlPointIndicesCount(); i++) {
			int index = cluster->GetControlPointIndices()[i];
			double weight = cluster->GetControlPointWeights()[i];
			FbxMatrix influence = vertexTransformMatrix * weight;
			clusterDeformation[index] += influence;
		}
	}

	// �ŏI�I�Ȓ��_���W���v�Z��VERTEX�ɕϊ�
	for (int i = 0; i < m_mesh->GetControlPointsCount(); i++) {
		FbxVector4 outVertex = clusterDeformation[i].MultNormalize(m_mesh->GetControlPointAt(i));
		vertices[i].Pos.x = (FLOAT)outVertex[0];
		vertices[i].Pos.y = (FLOAT)outVertex[1];
		vertices[i].Pos.z = (FLOAT)outVertex[2];
	}

	delete[] clusterDeformation;
	// ---------------------


	D3D11_MAPPED_SUBRESOURCE pdata;
	CONSTANT_BUFFER cb;
	// �p�����[�^�̎󂯓n��(�萔)
	cb.mWVP = DirectX::XMMatrixTranspose(world * view * proj);
	context->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pdata);
	memcpy_s(pdata.pData, pdata.RowPitch, (void*)(&cb), sizeof(cb));
	context->Unmap(m_constantBuffer, 0);

	// �p�����[�^�̎󂯓n��(���_)
	context->Map(VerBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &pdata);
	memcpy_s(pdata.pData, pdata.RowPitch, (void*)(vertices), sizeof(VERTEX) * m_mesh->GetControlPointsCount());
	context->Unmap(VerBuffer, 0);

	// �`����s
	context->DrawIndexed(m_mesh->GetPolygonVertexCount(), 0, 0);
}

void FBXSDK_Helper::FBX_Model::Create(
	HWND hwnd,
	ID3D11Device* device,
	ID3D11DeviceContext* context,
	ID3D11RenderTargetView* renderTargetView,
	const char* fbxfile_path)
{
	// <�p�C�v���C������>
	RECT csize;
	GetClientRect(hwnd, &csize);
	int CWIDTH = csize.right;
	int CHEIGHT = csize.bottom;

	// <�r���[�|�[�g�̐ݒ�>
	D3D11_VIEWPORT vp;
	vp.Width = CWIDTH;
	vp.Height = CHEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

	// <�V�F�[�_�̐ݒ�>
	ID3DBlob *pCompileVS = NULL;
	ID3DBlob *pCompilePS = NULL;
	D3DCompileFromFile(L"shader.hlsl", NULL, NULL, "VS", "vs_5_0", NULL, 0, &pCompileVS, NULL);
	assert(pCompileVS && "pCompileVS is nullptr !");
	(device->CreateVertexShader(pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), NULL, &pVertexShader));
	D3DCompileFromFile(L"shader.hlsl", NULL, NULL, "PS", "ps_5_0", NULL, 0, &pCompilePS, NULL);
	assert(pCompileVS && "pCompilePS is nullptr !");
	(device->CreatePixelShader(pCompilePS->GetBufferPointer(), pCompilePS->GetBufferSize(), NULL, &pPixelShader));

	// <���_���C�A�E�g>
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	(device->CreateInputLayout(layout, 1, pCompileVS->GetBufferPointer(), pCompileVS->GetBufferSize(), &pVertexLayout));
	pCompileVS->Release();
	pCompilePS->Release();

	// <�萔�o�b�t�@�̐ݒ�>
	D3D11_BUFFER_DESC cb;
	cb.ByteWidth = sizeof(CONSTANT_BUFFER);
	cb.Usage = D3D11_USAGE_DYNAMIC;
	cb.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cb.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cb.MiscFlags = 0;
	cb.StructureByteStride = 0;
	(device->CreateBuffer(&cb, NULL, &m_constantBuffer));

	// <FBX�ǂݍ���>
	FBX_Import(fbxfile_path);
	// <���_�f�[�^�̎��o��>
	//FBX_GetVertex();
	// <���_�f�[�^�p�o�b�t�@�̐ݒ�>
	FBX_SetVertexData(device);

	// <���X�^���C�U�̐ݒ�>
	D3D11_RASTERIZER_DESC rdc = {};
	rdc.CullMode = D3D11_CULL_BACK;
	rdc.FillMode = D3D11_FILL_SOLID;
	rdc.FrontCounterClockwise = TRUE;
	(device->CreateRasterizerState(&rdc, &pRasterizerState));

	// <�I�u�W�F�N�g�̔��f>
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
	// <FBX�ǂݍ���>
	m_fbxManager = FbxManager::Create();
	m_fbxScene = FbxScene::Create(m_fbxManager, "fbxscene");
	FbxString FileName(fbxfile_path);
	FbxImporter *fbxImporter = FbxImporter::Create(m_fbxManager, "imp");
	fbxImporter->Initialize(FileName.Buffer(), -1, m_fbxManager->GetIOSettings());
	fbxImporter->Import(m_fbxScene);
	fbxImporter->Destroy();

	FbxArray<FbxString*> AnimStackNameArray;
	m_fbxScene->FillAnimStackNameArray(AnimStackNameArray);
	FbxAnimStack *AnimationStack = m_fbxScene->FindMember<FbxAnimStack>(AnimStackNameArray[AnimStackNumber]->Buffer());
	m_fbxScene->SetCurrentAnimationStack(AnimationStack);

	FbxTakeInfo *takeInfo = m_fbxScene->GetTakeInfo(*(AnimStackNameArray[AnimStackNumber]));
	start = takeInfo->mLocalTimeSpan.GetStart();
	stop = takeInfo->mLocalTimeSpan.GetStop();
	FrameTime.SetTime(0, 0, 0, 1, 0, m_fbxScene->GetGlobalSettings().GetTimeMode());
	timeCount = start;

	for (int i = 0; i < m_fbxScene->GetRootNode()->GetChildCount(); i++) {
		if (m_fbxScene->GetRootNode()->GetChild(i)->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh) {
			m_meshNode = m_fbxScene->GetRootNode()->GetChild(i);
			m_mesh = m_meshNode->GetMesh();
			break;
		}
	}
}

void FBXSDK_Helper::FBX_Model::FBX_GetVertex()
{
	// <���_�f�[�^�̎��o��>
	for (int i = 0; i < m_fbxScene->GetRootNode()->GetChildCount(); i++) {
		if (m_fbxScene->GetRootNode()->GetChild(i)->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh) {
			m_mesh = m_fbxScene->GetRootNode()->GetChild(i)->GetMesh();
			break;
		}
	}
	vertices = new VERTEX[m_mesh->GetControlPointsCount()];
	for (int i = 0; i < m_mesh->GetControlPointsCount(); i++) {
		vertices[i].Pos.x = static_cast<FLOAT>(m_mesh->GetControlPointAt(i)[0]);
		vertices[i].Pos.y = static_cast<FLOAT>(m_mesh->GetControlPointAt(i)[1]);
		vertices[i].Pos.z = static_cast<FLOAT>(m_mesh->GetControlPointAt(i)[2]);
	}
}

void FBXSDK_Helper::FBX_Model::FBX_SetVertexData(ID3D11Device* device)
{
	{
		D3D11_BUFFER_DESC bd_vertex;
		bd_vertex.ByteWidth = sizeof(VERTEX) * m_mesh->GetControlPointsCount();
		bd_vertex.Usage = D3D11_USAGE_DYNAMIC;
		bd_vertex.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd_vertex.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bd_vertex.MiscFlags = 0;
		bd_vertex.StructureByteStride = 0;
		if (FAILED(device->CreateBuffer(&bd_vertex, NULL, &VerBuffer))) {
			assert(false && "Missing !");
		}
		vertices = new VERTEX[m_mesh->GetControlPointsCount()];

		// �C���f�b�N�X�f�[�^�̎��o���ƃo�b�t�@�̐ݒ�
		D3D11_BUFFER_DESC bd_index;
		bd_index.ByteWidth = sizeof(int) * m_mesh->GetPolygonVertexCount();
		bd_index.Usage = D3D11_USAGE_DEFAULT;
		bd_index.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd_index.CPUAccessFlags = 0;
		bd_index.MiscFlags = 0;
		bd_index.StructureByteStride = 0;
		D3D11_SUBRESOURCE_DATA data_index;
		data_index.pSysMem = m_mesh->GetPolygonVertices();
		(device->CreateBuffer(&bd_index, &data_index, &IndBuffer));
	}
}

