#pragma once

// <FBXのパース用クラス>

// 2019 : 09 : 19

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#ifndef FBXSDK_NEW_API
#define FBXSDK_NEW_API	// <新しいバージョンを使うとき用>
#endif

#include <fbxsdk.h>
#include <Windows.h>

// <UVSet名, 頂点内のUVセット順序>
using UVsetID = std::tr1::unordered_map<std::string, int>;
// <UVSet名, テクスチャパス名(一つのUVSetに複数テクスチャがぶら下がっていることがある>
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
		// <FBXのマテリアルは, Lambert, Phong しかない>
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
		float					TransparencyFactor;	// <透過度>
	};

	// <メッシュ構成要素>
	struct MESH_ELEMENTS
	{
		unsigned int			numPosition;	// <頂点座標のセットをいくつ持つか>
		unsigned int			numNormal;		// 
		unsigned int			numUVSet;		// <UVセット数>
	};

	struct FBX_MESH_NODE
	{
		std::string				name;			// <ノード名>
		std::string				parentName;		// <親ノード名(親がいないなら"null"という名前になる)>

		MESH_ELEMENTS			elements;		// <メッシュが保持するデータ構造>
		std::vector<FBX_MATERIAL_NODE>	m_materialArray;	// マテリアル
		UVsetID					uvsetID;

		std::vector<unsigned int>		indexArray;			// インデックス配列
		std::vector<FbxVector4>			m_positionArray;	// ポジション配列
		std::vector<FbxVector4>			m_normalArray;		// 法線配列
		std::vector<FbxVector2>			m_texcoordArray;	// テクスチャ座標配列

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

		// <読み込み>
		HRESULT LoadFBX(const char* file_name, const eAXIS_SYSTEM axis);
		FbxNode& GetRootNode();

		size_t GetNodesCount() { return m_meshNodeArray.size(); };

		FBX_MESH_NODE& GetNode(const unsigned int id);
	};
}
// FBX_LOADER