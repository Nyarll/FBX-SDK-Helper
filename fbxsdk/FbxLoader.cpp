#include "FbxLoader.h"


bool FBX_LOADER::FbxLoader::Load(const char * file_name)
{
	// <�t�@�C�����`�F�b�N>
	if (!file_name)
		return false;

	// <FBX �}�l�[�W���̐����E�`�F�b�N>
	m_fbxManager.reset(fbxsdk::FbxManager::Create());
	if (!m_fbxManager.get())
		return false;

	// <�C���|�[�^�̍쐬>
	m_fbxImporter.reset(fbxsdk::FbxImporter::Create(m_fbxManager.get(), "FbxObject"));

	// <�t�@�C���t�H�[�}�b�g�`�F�b�N>
	int lFileFormat = -1;
	if (!m_fbxManager->GetIOPluginRegistry()->DetectReaderFileFormat(file_name, lFileFormat))
	{
		lFileFormat = m_fbxManager->GetIOPluginRegistry()->FindReaderIDByDescription("FBX binary (*.fbx)");
	}

	// <�C���|�[�^������>
	if (!m_fbxImporter.get() || m_fbxImporter->Initialize(file_name, lFileFormat) == false)
		return false;

	// <���f���ǂݍ���>
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
		// <�q�m�[�h��T��>
		TriangulateRecursive(pNode->GetChild(lChildIndex));
	}
}