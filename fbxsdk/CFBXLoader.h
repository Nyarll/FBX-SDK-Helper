#pragma once

// <FBX�̃p�[�X�p�N���X>

// 2019 : 09 : 19

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#ifndef FBXSDK_NEW_API
#define FBXSDK_NEW_API	// <�V�����o�[�W�������g���Ƃ��p>
#endif

#include <fbxsdk.h>
#include <Windows.h>

// <UVSet��, ���_����UV�Z�b�g����>
using UVsetID = std::tr1::unordered_map<std::string, int>;
// <UVSet��, �e�N�X�`���p�X��(���UVSet�ɕ����e�N�X�`�����Ԃ牺�����Ă��邱�Ƃ�����>
using TextureSet = std::tr1::unordered_map<std::string, std::vector<std::string>>;

namespace FBX_LOADER
{
	struct FBX_MATERIAL_ELEMENT
	{
		enum MATERIAL_ELEMENT_TYPE
		{
			ELEMENT_NONE = 0,
			ELEMENT_COLOR,
			ELEMENT_TEXTURE,
			ELEMENT_BOTH,
			ELEMENT_MAX,
		};

		MATERIAL_ELEMENT_TYPE	type;
		float					r, g, b, a;
		TextureSet				textureSetArray;

		FBX_MATERIAL_ELEMENT()
		{
			textureSetArray.clear();
		}

		~FBX_MATERIAL_ELEMENT()
		{
			this->Release();
		}

		void Release()
		{
			for (auto& tex : textureSetArray)
			{
				tex.second.clear();
			}
			textureSetArray.clear();
		}
	};

	struct FBX_MATERIAL_NODE
	{
		// <FBX�̃}�e���A����, Lambert, Phong �����Ȃ�>
		enum eMATERIAL_TYPE
		{
			MATERIAL_LAMBERT = 0,
			MATERIAL_PHONG,
		};

		eMATERIAL_TYPE			type;
		FBX_MATERIAL_ELEMENT	ambient;
		FBX_MATERIAL_ELEMENT	diffuse;
		FBX_MATERIAL_ELEMENT	emmisive;
		FBX_MATERIAL_ELEMENT	specular;

		float					shininess;
		float					TransparencyFactor;	// <���ߓx>
	};

	// <���b�V���\���v�f>
	struct MESH_ELEMENTS
	{
		unsigned int			numPosition;	// <���_���W�̃Z�b�g����������>
		unsigned int			numNormal;		// 
		unsigned int			numUVSet;		// <UV�Z�b�g��>
	};

	struct FBX_MESH_NODE
	{
		std::string				name;			// <�m�[�h��>
		std::string				parentName;		// <�e�m�[�h��(�e�����Ȃ��Ȃ�"null"�Ƃ������O�ɂȂ�)>

		MESH_ELEMENTS			elements;		// <���b�V�����ێ�����f�[�^�\��>
		std::vector<FBX_MATERIAL_NODE>	m_materialArray;	// �}�e���A��
		UVsetID					uvsetID;

		std::vector<unsigned int>		indexArray;			// �C���f�b�N�X�z��
		std::vector<FbxVector4>			m_positionArray;	// �|�W�V�����z��
		std::vector<FbxVector4>			m_normalArray;		// �@���z��
		std::vector<FbxVector2>			m_texcoordArray;	// �e�N�X�`�����W�z��

		float					mat4x4[16];	// Matrix

		~FBX_MESH_NODE()
		{
			Release();
		}

		void Release()
		{
			uvsetID.clear();
			m_texcoordArray.clear();
			m_materialArray.clear();
			indexArray.clear();
			m_positionArray.clear();
			m_normalArray.clear();
		}
	};

	class CFBXLoader
	{
	public:
		enum eAXIS_SYSTEM
		{
			eAXIS_DIRECTX = 0,
			eAXIS_OPENGL,
		};

	protected:
		// <FBX SDK>
		FbxManager*		mSdkManager;
		FbxScene*		mScene;
		FbxImporter*	mImporter;
		FbxAnimLayer*	mCurrentAnimLayer;

		std::vector<FBX_MESH_NODE>	m_meshNodeArray;

		void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene);
		void TriangulateRecursive(FbxNode* pNode);

		void SetupNode(FbxNode* pNode, std::string parentName);
		void Setup();

		void CopyVertexData(FbxMesh* pMesh, FBX_MESH_NODE* meshNode);
		void CopyMaterialData(FbxSurfaceMaterial* mat, FBX_MATERIAL_NODE* destMat);

		void ComputeNodeMatrix(FbxNode* pNode, FBX_MESH_NODE* meshNode);

		void SetFbxColor(FBX_MATERIAL_ELEMENT& destColor, const FbxDouble3 srcColor);
		FbxDouble3 GetMaterialProperty(
			const FbxSurfaceMaterial * pMaterial,
			const char * pPropertyName,
			const char * pFactorPropertyName,
			FBX_MATERIAL_ELEMENT*			pElement);

		static void FBXMatrixToFloat16(FbxMatrix* src, float dest[16])
		{
			unsigned int nn = 0;
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					dest[nn] = static_cast<float>(src->Get(i, j));
					nn++;
				}
			}
		}

	public:
		CFBXLoader();
		~CFBXLoader();

		void Release();

		// <�ǂݍ���>
		HRESULT LoadFBX(const char* file_name, const eAXIS_SYSTEM axis);
		FbxNode& GetRootNode();

		size_t GetNodesCount() { return m_meshNodeArray.size(); };

		FBX_MESH_NODE& GetNode(const unsigned int id);
	};
}
// FBX_LOADER