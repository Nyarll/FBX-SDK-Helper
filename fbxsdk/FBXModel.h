#pragma once

// <FBX SDK �w���p�[�N���X>
// <�쐬�� : 2019 / 08 / 02>
// <�쐬�� : Nyarll>
// GitHub : https://github.com/Nyarll

// <FBX�t�@�C�����璼�ڃ��f����\�����邽�߂�SDK>
// <�_�E�����[�h�y�[�W>
// https://www.autodesk.com/developer-network/platform-technologies/fbx-sdk-2019-5
// <�Q�l�T�C�g>
// https://www.tkng45memo.com/fbxmesh
// <�ȉ��̏ꏊ�� FBX SDK ���C���X�g�[�����܂��傤(�f�t�H���g�̏ꏊ)>
// C:\Program Files\Autodesk\FBX\FBX SDK\2019.5
// DirectXTK�Ɠ��l�ɁA�ǉ��̃C���N���[�h�t�@�C���Ȃǂ̎w������܂��傤(�Q�l�T�C�g������΂����炭�������܂�)

// <�t���i(?)�̃V�F�[�_�[�t�@�C��[shader.hlsl]�ɂ���>
// ���̕ύX(���ȐӔC)
// Visual Studio ����A shader.hlsl �̃v���p�e�B -> HLSL�R���p�C�� -> �S��
// �S�ʃ^�u���� �V�F�[�_�[�̎�� : �G�t�F�N�g(/fx) , �V�F�[�_�[���f�� : Shader Model 5.0(/5_0)
// �ɐݒ肵�܂��傤

// <include>
#include <DirectXMath.h>
#include <SimpleMath.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <fbxsdk.h>

// <link FBX SDK library>
// -mt(�}���`�X���b�h�f�o�b�O(MTd))
#pragma comment(lib, "libfbxsdk-mt.lib")
#pragma comment(lib, "zlib-mt.lib")
#pragma comment(lib, "libxml2-mt.lib")
#pragma comment(lib, "libfbxsdk.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace FBXSDK_Helper
{
	// <FBX Model class>
	class FBX_Model
	{
	protected:
		// <��̒��_�����i�[����\����>
		struct VERTEX {
			DirectX::XMFLOAT3 Pos;
		};

		// <GPU(�V�F�[�_��)�֑��鐔�l���܂Ƃ߂��\����>
		struct CONSTANT_BUFFER {
			DirectX::XMMATRIX mWVP;
		};

	protected:
		ID3D11RasterizerState *pRasterizerState;
		ID3D11VertexShader *pVertexShader;
		ID3D11InputLayout *pVertexLayout;
		ID3D11PixelShader *pPixelShader;
		ID3D11Buffer *m_constantBuffer;

		FbxManager *m_fbxManager = nullptr;
		FbxScene *m_fbxScene = NULL;
		FbxNode* m_meshNode = NULL;
		FbxMesh *m_mesh = NULL;
		ID3D11Buffer *VerBuffer = NULL;
		ID3D11Buffer *IndBuffer = NULL;
		VERTEX *vertices;

		int AnimStackNumber = 0;
		FbxTime FrameTime, timeCount, start, stop;

	public:
		FBX_Model();
		~FBX_Model();

		// <�`��>
		virtual void Draw(
			ID3D11DeviceContext1* context,
			DirectX::SimpleMath::Matrix world,
			DirectX::SimpleMath::Matrix view,
			DirectX::SimpleMath::Matrix proj);

		void Draw(
			ID3D11DeviceContext* context,
			DirectX::SimpleMath::Matrix& world,
			DirectX::SimpleMath::Matrix& view,
			DirectX::SimpleMath::Matrix& proj);

		// <���f���쐬>
		virtual void Create(
			HWND hwnd,
			ID3D11Device* device,
			ID3D11DeviceContext* context,
			ID3D11RenderTargetView* renderTargetView,
			const char* fbxfile_path);

		// <�j��>
		void Destroy();

	protected:
		virtual void FBX_Import(const char* fbxfile_path);
		virtual void FBX_GetVertex();
		virtual void FBX_SetVertexData(ID3D11Device1* device);
	};

	class FBX_AnimationModel : public FBX_Model
	{
	private:
		FbxNode* m_meshNode = NULL;

		int AnimStackNumber = 0;
		FbxTime FrameTime, timeCount, start, stop;
	public:
		FBX_AnimationModel();
		~FBX_AnimationModel();

		// <�`��>
		virtual void Draw(
			ID3D11DeviceContext1* context,
			DirectX::SimpleMath::Matrix world,
			DirectX::SimpleMath::Matrix view,
			DirectX::SimpleMath::Matrix proj)override;

		// <���f���쐬>
		virtual void Create(
			HWND hwnd,
			ID3D11Device1* device,
			ID3D11DeviceContext1* context,
			ID3D11RenderTargetView* renderTargetView,
			const char* fbxfile_path)override;

	protected:
		virtual void FBX_Import(const char* fbxfile_path)override;
		virtual void FBX_SetVertexData(ID3D11Device1* device)override;

	};
}