#include "FbxLoader.h"


bool FBX_LOADER::FbxLoader::Load(const char * file_name)
{
	// <ファイル名チェック>
	if (!file_name)
		return false;

	// <FBX マネージャの生成・チェック>
	m_fbxManager.reset(fbxsdk::FbxManager::Create());
	if (!m_fbxManager.get())
		return false;

	// <インポータの作成>
	m_fbxImporter.reset(fbxsdk::FbxImporter::Create(m_fbxManager.get(), "FbxObject"));

	// <ファイルフォーマットチェック>
	int lFileFormat = -1;
	if (!m_fbxManager->GetIOPluginRegistry()->DetectReaderFileFormat(file_name, lFileFormat))
	{
		lFileFormat = m_fbxManager->GetIOPluginRegistry()->FindReaderIDByDescription("FBX binary (*.fbx)");
	}

	// <インポータ初期化>
	if (!m_fbxImporter.get() || m_fbxImporter->Initialize(file_name, lFileFormat) == false)
		return false;

	// <モデル読み込み>
	if (!m_fbxImporter.get() || m_fbxImporter->Import(m_scene.get()) == false)
		return false;

	FbxSystemUnit SceneSystemUnit = m_scene->GetGlobalSettings().GetSystemUnit();
	if (SceneSystemUnit.GetScaleFactor() != 1.0)
	{
		FbxSystemUnit::cm.ConvertScene(m_scene.get());
	}

	this->TriangulateRecursive(m_scene->GetRootNode());

	return true;
}

void FBX_LOADER::FbxLoader::TriangulateRecursive(FbxNode* pNode)
{
	FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();

	if (lNodeAttribute)
	{
		if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh ||
			lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNurbs ||
			lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNurbsSurface ||
			lNodeAttribute->GetAttributeType() == FbxNodeAttribute::ePatch)
		{
			FbxGeometryConverter lConverter(pNode->GetFbxManager());
			lConverter.Triangulate(m_scene.get(), true);
		}
	}

	const int lChildCount = pNode->GetChildCount();
	for (int lChildIndex = 0; lChildIndex < lChildCount; ++lChildIndex)
	{
		// <子ノードを探索>
		TriangulateRecursive(pNode->GetChild(lChildIndex));
	}
}