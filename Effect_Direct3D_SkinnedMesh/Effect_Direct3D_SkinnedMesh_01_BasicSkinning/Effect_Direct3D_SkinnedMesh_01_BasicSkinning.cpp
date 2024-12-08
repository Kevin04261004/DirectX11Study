// Effect_Direct3D_SkinnedMesh_01_BasicSkinning.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "Effect_Direct3D_SkinnedMesh_01_BasicSkinning.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	// 디버그 빌드 할때 CRT를 이용한 런타임 메모리릭 찾기를 수행
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	CK_DX_BasicSkinnedMesh theApp(hInstance);

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

CK_DX_BasicSkinnedMesh::CK_DX_BasicSkinnedMesh(HINSTANCE hInstance)
	: D3DApp(hInstance), mCharacterModel(0)
{
	mMainWndCaption = L"SkinnedMesh Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	mCam.SetPosition(0.0f, 2.0f, -15.0f);

	mDirLights[0].Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(1.0f, 0.9f, 0.9f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
	mDirLights[0].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.40f, 0.40f, 0.40f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[1].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	mDirLights[2].Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, 0.0, -1.0f);
}

CK_DX_BasicSkinnedMesh::~CK_DX_BasicSkinnedMesh()
{
	SafeDelete(mCharacterModel);
	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool CK_DX_BasicSkinnedMesh::Init()
{
	if (!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);
	RenderStates::InitAll(md3dDevice);

	mTexMgr.Init(md3dDevice);

	mCam.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);

	mCharacterModel = new SkinnedModel(md3dDevice, mTexMgr, "Models\\soldier.m3d", L"Textures\\");
	// Reflect to change coordinate system from the RHS the data was exported out as.
	XMMATRIX modelScale = XMMatrixScaling(0.05f, 0.05f, -0.05f);
	XMMATRIX modelRot = XMMatrixRotationY(MathHelper::Pi);
	XMMATRIX modelOffset = XMMatrixTranslation(-2.0f, 0.0f, -7.0f);
	for (int i = 0; i < 10; ++i)
	{
		mCharacterInstance[i].Model = mCharacterModel;
		mCharacterInstance[i].TimePos = 0.0f;
		mCharacterInstance[i].ClipName = "Take1";
		mCharacterInstance[i].FinalTransforms.resize(mCharacterModel->SkinnedData.BoneCount());
		XMStoreFloat4x4(&mCharacterInstance[i].World, modelScale * modelRot * modelOffset);

		modelOffset = XMMatrixTranslation(2.0f, 0.0f, 7.0f * i);
		XMStoreFloat4x4(&mCharacterInstance[i].World, modelScale * modelRot * modelOffset);
	}

	return true;
}

void CK_DX_BasicSkinnedMesh::OnResize()
{
	D3DApp::OnResize();

	mCam.SetLens(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}

void CK_DX_BasicSkinnedMesh::UpdateScene(float dt)
{
	//
	// Control the camera.
	//
	if (GetAsyncKeyState('W') & 0x8000)
		mCam.Walk(10.0f * dt);

	if (GetAsyncKeyState('S') & 0x8000)
		mCam.Walk(-10.0f * dt);

	if (GetAsyncKeyState('A') & 0x8000)
		mCam.Strafe(-10.0f * dt);

	if (GetAsyncKeyState('D') & 0x8000)
		mCam.Strafe(10.0f * dt);

	//
	// Animate the character.
	// 

	for (int i = 0; i < 10; ++i)
	{
		mCharacterInstance[i].Update(dt);
	}

	mCam.UpdateViewMatrix();
}

void CK_DX_BasicSkinnedMesh::DrawScene()
{
	// 2023.12.09 
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::DimGray));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	md3dImmediateContext->RSSetViewports(1, &mScreenViewport);

	XMMATRIX view = mCam.View();
	XMMATRIX proj = mCam.Proj();
	XMMATRIX viewProj = mCam.ViewProj();
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	Effects::NormalMapFX->SetDirLights(mDirLights);
	Effects::NormalMapFX->SetEyePosW(mCam.GetPosition());

	ID3DX11EffectTechnique* activeSkinnedTech = Effects::NormalMapFX->Light3TexSkinnedTech;
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMMATRIX world;
	XMMATRIX worldInvTranspose;
	XMMATRIX worldViewProj;

	//
	// Draw the animated characters.
	//

	md3dImmediateContext->IASetInputLayout(InputLayouts::PosNormalTexTanSkinned);

	D3DX11_TECHNIQUE_DESC techDesc;
	activeSkinnedTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		for (int i = 0; i < 5; ++i)
		{
			world = XMLoadFloat4x4(&mCharacterInstance[i].World);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world * view * proj;

			Effects::NormalMapFX->SetWorld(world);
			Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
			Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));
			Effects::NormalMapFX->SetBoneTransforms(&mCharacterInstance[i].FinalTransforms[0], mCharacterInstance[i].FinalTransforms.size());

			for (UINT subset = 0; subset < mCharacterInstance[i].Model->SubsetCount; ++subset)
			{
				Effects::NormalMapFX->SetMaterial(mCharacterInstance[i].Model->Mat[subset]);
				Effects::NormalMapFX->SetDiffuseMap(mCharacterInstance[i].Model->DiffuseMapSRV[subset]);
				Effects::NormalMapFX->SetNormalMap(mCharacterInstance[i].Model->NormalMapSRV[subset]);

				activeSkinnedTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
				mCharacterInstance[i].Model->ModelMesh.Draw(md3dImmediateContext, subset);
			}
		}

		md3dImmediateContext->RSSetState(0);
		md3dImmediateContext->RSSetState(RenderStates::WireframeRS);

		for (int i = 5; i < 10; ++i)
		{
			world = XMLoadFloat4x4(&mCharacterInstance[i].World);
			worldInvTranspose = MathHelper::InverseTranspose(world);
			worldViewProj = world * view * proj;

			Effects::NormalMapFX->SetWorld(world);
			Effects::NormalMapFX->SetWorldInvTranspose(worldInvTranspose);
			Effects::NormalMapFX->SetWorldViewProj(worldViewProj);
			Effects::NormalMapFX->SetTexTransform(XMMatrixScaling(1.0f, 1.0f, 1.0f));
			Effects::NormalMapFX->SetBoneTransforms(&mCharacterInstance[i].FinalTransforms[0], mCharacterInstance[i].FinalTransforms.size());

			for (UINT subset = 0; subset < mCharacterInstance[i].Model->SubsetCount; ++subset)
			{
				Effects::NormalMapFX->SetMaterial(mCharacterInstance[i].Model->Mat[subset]);
				Effects::NormalMapFX->SetDiffuseMap(mCharacterInstance[i].Model->DiffuseMapSRV[subset]);
				Effects::NormalMapFX->SetNormalMap(mCharacterInstance[i].Model->NormalMapSRV[subset]);

				activeSkinnedTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
				mCharacterInstance[i].Model->ModelMesh.Draw(md3dImmediateContext, subset);
			}
		}
	}

	// Turn off wireframe.
	md3dImmediateContext->RSSetState(0);

	// Restore from RenderStates::EqualsDSS
	md3dImmediateContext->OMSetDepthStencilState(0, 0);

	HR(mSwapChain->Present(0, 0));
}

void CK_DX_BasicSkinnedMesh::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void CK_DX_BasicSkinnedMesh::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CK_DX_BasicSkinnedMesh::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		mCam.Pitch(dy);
		mCam.RotateY(dx);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

