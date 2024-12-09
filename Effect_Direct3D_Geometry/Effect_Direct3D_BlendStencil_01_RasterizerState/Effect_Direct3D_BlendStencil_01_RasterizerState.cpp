﻿// Effect_Direct3D_BlendStencil_01_RasterizerState.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "Effect_Direct3D_BlendStencil_01_RasterizerState.h"


// WinMain 함수 
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{

	// 디버그 빌드 할때 CRT를 이용한 런타임 메모리릭 찾기를 수행
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	CK_DX_RasterizerStateBox theApp(hInstance);

	/*char* cp;
	cp = (char*)malloc(16);*/

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

CK_DX_RasterizerStateBox::CK_DX_RasterizerStateBox(HINSTANCE hInstance)
	: D3DApp(hInstance), mBoxVB(0), mBoxIB(0), mDiffuseMapSRV(0), mEyePosW(0.0f, 0.0f, 0.0f),
	mTheta(1.3f * MathHelper::Pi), mPhi(0.4f * MathHelper::Pi), mRadius(2.5f)
{
	mMainWndCaption = L"CKDK_Rasterizer Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mBoxWorld, I);
	XMStoreFloat4x4(&mTexTransform, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	mDirLights[0].Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
	mDirLights[0].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);

	mDirLights[1].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights[1].Diffuse = XMFLOAT4(1.4f, 1.4f, 1.4f, 1.0f);
	mDirLights[1].Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
	mDirLights[1].Direction = XMFLOAT3(-0.707f, 0.0f, 0.707f);

	mBoxMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
}

CK_DX_RasterizerStateBox::~CK_DX_RasterizerStateBox()
{
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	ReleaseCOM(mDiffuseMapSRV);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool CK_DX_RasterizerStateBox::Init()
{
	if (!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);
	RenderStates::InitAll(md3dDevice);

	// https://github.com/walbourn/directx-sdk-samples 에서 dds 파일 로더 부분 가지고 옴
	//HR(CreateDDSTextureFromFile(md3dDevice, L"Textures/WoodCrate01.dds", nullptr, &mDiffuseMapSRV));
	HR(CreateWICTextureFromFile(md3dDevice, L"Textures/WoodCrateJPG.jpg", nullptr, &mDiffuseMapSRV));

	BuildGeometryBuffers();

	return true;
}

void CK_DX_RasterizerStateBox::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void CK_DX_RasterizerStateBox::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	mEyePosW = XMFLOAT3(x, y, z);
	mDeltaTime += dt;
	// Effects::BasicFX->SetTimeVal(mDeltaTime);

	/*XMMATRIX rotateMat = XMMatrixRotationY(1.f * dt);
	XMMATRIX mBoxMat = XMLoadFloat4x4(&mBoxWorld);*/
	//XMMATRIX rotateEndMat = XMMatrixMultiply(mBoxMat, rotateMat);

	// XMStoreFloat4x4(&mBoxWorld, rotateEndMat);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);
}

void CK_DX_RasterizerStateBox::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;
	float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view * proj;

	// Set per frame constants.
	Effects::BasicFX->SetDirLights(mDirLights);
	Effects::BasicFX->SetEyePosW(mEyePosW);

	// Light1과 Light2에 대한 부분과 texture까지 처리하기 위해서 Light2TexTech 사용함 
	//ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light2TexAlphaClipTech;
	//ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light2TexTech;
	ID3DX11EffectTechnique* activeTech = Effects::BasicFX->GeoOctahedronTech;
	//ID3DX11EffectTechnique* activeTech = Effects::BasicFX->TexGeoNormal2Tech;
	//ID3DX11EffectTechnique* activeTech = Effects::BasicFX->TexGeoExplodeTech;
	//ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light0TexTech;
	//ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light1TexTech;
	//ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light1Tech;
	//ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light2Tech;


	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

		// Draw the box.
		XMMATRIX world = XMLoadFloat4x4(&mBoxWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mTexTransform));
		Effects::BasicFX->SetMaterial(mBoxMat);
		Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRV);

		//md3dImmediateContext->RSSetState(RenderStates::WireframeRS);
		//md3dImmediateContext->RSSetState(RenderStates::CullRS);
		//md3dImmediateContext->RSSetState(RenderStates::NoCullWireframeRS);
		md3dImmediateContext->RSSetState(RenderStates::NoCullRS);		

		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

		md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

		// Restore default render state.
		md3dImmediateContext->RSSetState(0);
	}


	//activeTech = Effects::BasicFX->TexGeoExplodeTech;

	//activeTech->GetDesc(&techDesc);
	//for (UINT p = 0; p < techDesc.Passes; ++p)
	//{
	//	md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
	//	md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

	//	// Draw the box.
	//	XMMATRIX world = XMLoadFloat4x4(&mBoxWorld);
	//	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	//	XMMATRIX worldViewProj = world * view * proj;

	//	Effects::BasicFX->SetWorld(world);
	//	Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
	//	Effects::BasicFX->SetWorldViewProj(worldViewProj);
	//	Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mTexTransform));
	//	Effects::BasicFX->SetMaterial(mBoxMat);
	//	Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRV);

	//	//md3dImmediateContext->RSSetState(RenderStates::WireframeRS);
	//	//md3dImmediateContext->RSSetState(RenderStates::CullRS);
	//	md3dImmediateContext->RSSetState(RenderStates::NoCullWireframeRS);
	//	//md3dImmediateContext->RSSetState(RenderStates::NoCullRS);

	//	activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

	//	md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

	//	// Restore default render state.
	//	md3dImmediateContext->RSSetState(0);

	//}

	HR(mSwapChain->Present(0, 0));
}

void CK_DX_RasterizerStateBox::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void CK_DX_RasterizerStateBox::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CK_DX_RasterizerStateBox::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f * static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f * static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.01 unit in the scene.
		float dx = 0.01f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.01f * static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 1.0f, 15.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void CK_DX_RasterizerStateBox::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData box;

	GeometryGenerator geoGen;
	// geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	geoGen.CreateGrid(1, 1, 2, 2, box);


	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	mBoxVertexOffset = 0;

	// Cache the index count of each object.
	mBoxIndexCount = box.Indices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	mBoxIndexOffset = 0;

	UINT totalVertexCount = box.Vertices.size();

	UINT totalIndexCount = mBoxIndexCount;

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	std::vector<Vertex::Basic32> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = box.Vertices[i].Position;
		vertices[k].Normal = box.Vertices[i].Normal;
		vertices[k].Tex = box.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mBoxVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	std::vector<UINT> indices;
	indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mBoxIB));
}

