#include "CFBXRendererDX.h"
#include "DDSTextureLoader.h"
#include <locale.h>

namespace FBX_LOADER
{

	CFBXRenderDX11::CFBXRenderDX11()
	{
		m_pFBX = nullptr;
	}

	CFBXRenderDX11::~CFBXRenderDX11()
	{
		Release();
	}

	void CFBXRenderDX11::Release()
	{
		for (size_t i = 0; i < m_meshNodeArray.size(); i++)
		{
			m_meshNodeArray[i].Release();
		}
		m_meshNodeArray.clear();

		if (m_pFBX)
		{
			delete m_pFBX;
			m_pFBX = nullptr;
		}
	}

	HRESULT CFBXRenderDX11::LoadFBX(const char* filename, ID3D11Device*	pd3dDevice)
	{
		if (!filename || !pd3dDevice)
			return E_FAIL;

		HRESULT hr = S_OK;

		m_pFBX = new CFBXLoader;
		hr = m_pFBX->LoadFBX(filename, CFBXLoader::eAXIS_OPENGL);
		if (FAILED(hr))
			return hr;

		hr = CreateNodes(pd3dDevice);
		if (FAILED(hr))
			return hr;

		return hr;
	}

	HRESULT CFBXRenderDX11::CreateNodes(ID3D11Device*	pd3dDevice)
	{
		if (!pd3dDevice)
			return E_FAIL;

		HRESULT hr = S_OK;

		size_t nodeCoount = m_pFBX->GetNodesCount();
		if (nodeCoount == 0)
			return E_FAIL;

		for (size_t i = 0; i < nodeCoount; i++)
		{
			MESH_NODE meshNode;
			FBX_MESH_NODE fbxNode = m_pFBX->GetNode(static_cast<unsigned int>(i));

			VertexConstruction(pd3dDevice, fbxNode, meshNode);

			// <index buffer>
			meshNode.indexCount = static_cast<DWORD>(fbxNode.indexArray.size());
			meshNode.SetIndexBit(meshNode.indexCount);
			if (fbxNode.indexArray.size() > 0)
				hr = CreateIndexBuffer(pd3dDevice, &meshNode.m_pIB, &fbxNode.indexArray[0], static_cast<uint32_t>(fbxNode.indexArray.size()));

			memcpy(meshNode.mat4x4, fbxNode.mat4x4, sizeof(float) * 16);

			// <�}�e���A��>
			MaterialConstruction(pd3dDevice, fbxNode, meshNode);

			m_meshNodeArray.push_back(meshNode);
		}

		return hr;
	}

	HRESULT CFBXRenderDX11::CreateVertexBuffer(ID3D11Device*	pd3dDevice, ID3D11Buffer** pBuffer, void* pVertices, uint32_t stride, uint32_t vertexCount)
	{
		if (!pd3dDevice || stride == 0 || vertexCount == 0)
			return E_FAIL;

		HRESULT hr = S_OK;

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = stride * vertexCount;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));

		InitData.pSysMem = pVertices;

		hr = pd3dDevice->CreateBuffer(&bd, &InitData, pBuffer);
		if (FAILED(hr))
			return hr;

		return hr;
	}

	HRESULT CFBXRenderDX11::CreateIndexBuffer(ID3D11Device*	pd3dDevice, ID3D11Buffer** pBuffer, void* pIndices, uint32_t indexCount)
	{
		if (!pd3dDevice || indexCount == 0)
			return E_FAIL;

		HRESULT hr = S_OK;
		size_t stride = sizeof(unsigned int);

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = static_cast<uint32_t>(stride*indexCount);
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));

		InitData.pSysMem = pIndices;

		hr = pd3dDevice->CreateBuffer(&bd, &InitData, pBuffer);
		if (FAILED(hr))
			return hr;

		return hr;
	}

	HRESULT CFBXRenderDX11::VertexConstruction(ID3D11Device*	pd3dDevice, FBX_MESH_NODE &fbxNode, MESH_NODE& meshNode)
	{
		meshNode.vertexCount = static_cast<DWORD>(fbxNode.m_positionArray.size());
		if (!pd3dDevice || meshNode.vertexCount == 0)
			return E_FAIL;

		HRESULT hr = S_OK;

		VERTEX_DATA*	pV = new VERTEX_DATA[meshNode.vertexCount];

		for (size_t i = 0; i < meshNode.vertexCount; i++)
		{
			FbxVector4 v = fbxNode.m_positionArray[i];
			pV[i].vPos = DirectX::XMFLOAT3((float)v.mData[0],
				(float)v.mData[1],
				(float)v.mData[2]);

			v = fbxNode.m_normalArray[i];

			pV[i].vNor = DirectX::XMFLOAT3((float)v.mData[0],
				(float)v.mData[1],
				(float)v.mData[2]);

			if ((float)fbxNode.m_texcoordArray.size() > 0)
			{
				// <�����UV1�������Ȃ�>
				// <UV��V�l���]>
				pV[i].vTexcoord = DirectX::XMFLOAT2((float)abs(1.0f - fbxNode.m_texcoordArray[i].mData[0]),
					(float)abs(1.0f - fbxNode.m_texcoordArray[i].mData[1]));
			}
			else
				pV[i].vTexcoord = DirectX::XMFLOAT2(0, 0);
		}

		CreateVertexBuffer(pd3dDevice, &meshNode.m_pVB, pV, sizeof(VERTEX_DATA), meshNode.vertexCount);

		if (pV)
			delete[] pV;

		return hr;
	}

	HRESULT CFBXRenderDX11::MaterialConstruction(ID3D11Device*	pd3dDevice, FBX_MESH_NODE &fbxNode, MESH_NODE& meshNode)
	{
		if (!pd3dDevice || fbxNode.m_materialArray.size() == 0)
			return E_FAIL;

		HRESULT hr = S_OK;

		// <����͐擪�̃}�e���A�������g��>
		FBX_MATERIAL_NODE fbxMaterial = fbxNode.m_materialArray[0];
		meshNode.materialData.specularPower = fbxMaterial.shininess;
		meshNode.materialData.TransparencyFactor = fbxMaterial.TransparencyFactor;

		meshNode.materialData.ambient
			= DirectX::XMFLOAT4(fbxMaterial.ambient.r, fbxMaterial.ambient.g, fbxMaterial.ambient.b, fbxMaterial.ambient.a);
		meshNode.materialData.diffuse
			= DirectX::XMFLOAT4(fbxMaterial.diffuse.r, fbxMaterial.diffuse.g, fbxMaterial.diffuse.b, fbxMaterial.diffuse.a);
		meshNode.materialData.specular
			= DirectX::XMFLOAT4(fbxMaterial.specular.r, fbxMaterial.specular.g, fbxMaterial.specular.b, fbxMaterial.specular.a);
		meshNode.materialData.emmisive
			= DirectX::XMFLOAT4(fbxMaterial.emmisive.r, fbxMaterial.emmisive.g, fbxMaterial.emmisive.b, fbxMaterial.emmisive.a);


		// <Diffuse��������e�N�X�`����ǂݍ���>
		if (fbxMaterial.diffuse.textureSetArray.size() > 0)
		{
			TextureSet::const_iterator it = fbxMaterial.diffuse.textureSetArray.begin();
			if (it->second.size())
			{
				std::string path = it->second[0];
		
				// June 2010�̎�����ύX
				// hr = D3DX11CreateShaderResourceViewFromFileA( pd3dDevice,path.c_str(), NULL, NULL, &meshNode.materialData.pSRV, NULL );
		
				// Todo: �b��Ή�
				// FBX SDK -> ������ : char ������ł�wchar�ɂ��Ȃ��Ƃ����Ȃ�
				WCHAR	wstr[512];
				size_t wLen = 0;
				mbstowcs_s(&wLen, wstr, path.size() + 1, path.c_str(), _TRUNCATE);
				CreateDDSTextureFromFile(pd3dDevice, wstr, NULL, &meshNode.materialData.pSRV, 0);
			}
		}

		// <samplerstate>
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		hr = pd3dDevice->CreateSamplerState(&sampDesc, &meshNode.materialData.pSampler);

		// <material Constant Buffer>
		D3D11_BUFFER_DESC bufDesc;
		ZeroMemory(&bufDesc, sizeof(bufDesc));
		bufDesc.ByteWidth = sizeof(MATERIAL_CONSTANT_DATA);
		bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufDesc.CPUAccessFlags = 0;

		hr = pd3dDevice->CreateBuffer(&bufDesc, NULL, &meshNode.materialData.pMaterialCb);

		meshNode.materialData.materialConstantData.ambient = meshNode.materialData.ambient;
		meshNode.materialData.materialConstantData.diffuse = meshNode.materialData.ambient;
		meshNode.materialData.materialConstantData.specular = meshNode.materialData.specular;
		meshNode.materialData.materialConstantData.emmisive = meshNode.materialData.emmisive;


		return hr;
	}

	//
	HRESULT CFBXRenderDX11::CreateInputLayout(ID3D11Device*	pd3dDevice, const void* pShaderBytecodeWithInputSignature, size_t BytecodeLength, D3D11_INPUT_ELEMENT_DESC* pLayout, unsigned int layoutSize)
	{
		// <InputeLayout�͒��_�V�F�[�_�̃R���p�C�����ʂ��K�v>
		if (!pd3dDevice || !pShaderBytecodeWithInputSignature || !pLayout)
			return E_FAIL;

		HRESULT hr = S_OK;

		size_t nodeCount = m_meshNodeArray.size();

		for (size_t i = 0; i < nodeCount; i++)
		{
			pd3dDevice->CreateInputLayout(pLayout, layoutSize, pShaderBytecodeWithInputSignature, BytecodeLength, &m_meshNodeArray[i].m_pInputLayout);
		}

		return hr;
	}

	HRESULT CFBXRenderDX11::RenderAll(ID3D11DeviceContext* pImmediateContext)
	{
		size_t nodeCount = m_meshNodeArray.size();
		if (nodeCount == 0)
			return S_OK;

		HRESULT hr = S_OK;

		pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		for (size_t i = 0; i < nodeCount; i++)
		{
			MESH_NODE* node = &m_meshNodeArray[i];

			if (node->vertexCount == 0)
				continue;

			UINT stride = sizeof(VERTEX_DATA);
			UINT offset = 0;
			pImmediateContext->IASetVertexBuffers(0, 1, &node->m_pVB, &stride, &offset);

			DXGI_FORMAT indexbit = DXGI_FORMAT_R16_UINT;
			if (node->m_indexBit == MESH_NODE::INDEX_32BIT)
				indexbit = DXGI_FORMAT_R32_UINT;

			pImmediateContext->IASetInputLayout(node->m_pInputLayout);
			pImmediateContext->IASetIndexBuffer(node->m_pIB, indexbit, 0);

			pImmediateContext->DrawIndexed(node->indexCount, 0, 0);
		}

		return hr;
	}

	HRESULT CFBXRenderDX11::RenderNode(ID3D11DeviceContext* pImmediateContext, const size_t nodeId)
	{
		size_t nodeCount = m_meshNodeArray.size();
		if (nodeCount == 0 || nodeCount <= nodeId)
			return S_OK;

		HRESULT hr = S_OK;

		MESH_NODE* node = &m_meshNodeArray[nodeId];

		if (node->vertexCount == 0)
			return S_OK;

		UINT stride = sizeof(VERTEX_DATA);
		UINT offset = 0;
		pImmediateContext->IASetVertexBuffers(0, 1, &node->m_pVB, &stride, &offset);
		pImmediateContext->IASetInputLayout(node->m_pInputLayout);
		pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// <�C���f�b�N�X�o�b�t�@�����݂���ꍇ>
		if (node->m_indexBit != MESH_NODE::INDEX_NOINDEX)
		{
			DXGI_FORMAT indexbit = DXGI_FORMAT_R16_UINT;
			if (node->m_indexBit == MESH_NODE::INDEX_32BIT)
				indexbit = DXGI_FORMAT_R32_UINT;

			pImmediateContext->IASetIndexBuffer(node->m_pIB, indexbit, 0);

			pImmediateContext->DrawIndexed(node->indexCount, 0, 0);
		}

		return hr;
	}

	HRESULT CFBXRenderDX11::RenderNodeInstancing(ID3D11DeviceContext* pImmediateContext, const size_t nodeId, const uint32_t InstanceCount)
	{
		size_t nodeCount = m_meshNodeArray.size();
		if (nodeCount == 0 || nodeCount <= nodeId || InstanceCount == 0)
			return S_OK;

		HRESULT hr = S_OK;

		MESH_NODE* node = &m_meshNodeArray[nodeId];

		if (node->vertexCount == 0)
			return S_OK;

		UINT stride = sizeof(VERTEX_DATA);
		UINT offset = 0;
		pImmediateContext->IASetVertexBuffers(0, 1, &node->m_pVB, &stride, &offset);
		pImmediateContext->IASetInputLayout(node->m_pInputLayout);
		pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// <�C���f�b�N�X�o�b�t�@�����݂���ꍇ>
		if (node->m_indexBit != MESH_NODE::INDEX_NOINDEX)
		{
			DXGI_FORMAT indexbit = DXGI_FORMAT_R16_UINT;
			if (node->m_indexBit == MESH_NODE::INDEX_32BIT)
				indexbit = DXGI_FORMAT_R32_UINT;

			pImmediateContext->IASetIndexBuffer(node->m_pIB, indexbit, 0);

			pImmediateContext->DrawIndexedInstanced(node->indexCount, InstanceCount, 0, 0, 0);
		}

		return hr;
	}

	HRESULT CFBXRenderDX11::RenderNodeInstancingIndirect(ID3D11DeviceContext* pImmediateContext, const size_t nodeId, ID3D11Buffer* pBufferForArgs, const uint32_t AlignedByteOffsetForArgs)
	{
		size_t nodeCount = m_meshNodeArray.size();
		if (nodeCount == 0 || nodeCount <= nodeId)
			return S_OK;

		HRESULT hr = S_OK;

		MESH_NODE* node = &m_meshNodeArray[nodeId];

		if (node->vertexCount == 0)
			return S_OK;

		UINT stride = sizeof(VERTEX_DATA);
		UINT offset = 0;
		pImmediateContext->IASetVertexBuffers(0, 1, &node->m_pVB, &stride, &offset);
		pImmediateContext->IASetInputLayout(node->m_pInputLayout);
		pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// <�C���f�b�N�X�o�b�t�@�����݂���ꍇ>
		if (node->m_indexBit != MESH_NODE::INDEX_NOINDEX)
		{
			DXGI_FORMAT indexbit = DXGI_FORMAT_R16_UINT;
			if (node->m_indexBit == MESH_NODE::INDEX_32BIT)
				indexbit = DXGI_FORMAT_R32_UINT;

			pImmediateContext->IASetIndexBuffer(node->m_pIB, indexbit, 0);

			pImmediateContext->DrawIndexedInstancedIndirect(pBufferForArgs, AlignedByteOffsetForArgs);
		}

		return hr;
	}

}
// FBX_LOADER