#pragma once

#include <DirectXMath.h>
#include <SimpleMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <fbxsdk.h>

namespace FBX_LOADER
{
	struct VERTEX
	{
		DirectX::XMFLOAT3 Pos;
	};

	class FbxLoader
	{
	private:
		std::unique_ptr<FbxManager> m_fbxManager;
		std::unique_ptr<FbxImporter> m_fbxImporter;

		std::unique_ptr<FbxScene> m_scene;

	public:
		bool Load(const char* file_name);

		void TriangulateRecursive(FbxNode* pNode);
	};
}