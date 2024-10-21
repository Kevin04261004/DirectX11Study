// Effect_Direct3D_Texture00_TextureFilter.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#pragma warning(disable : 4717)
#include "framework.h"
#include "Effect_Direct3D_Texture00_TextureFilter.h"

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

	CKDX_TextureFilterWithDirectXTK theApp(hInstance);

	/*char* cp;
	cp = (char*)malloc(16);*/

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

CKDX_TextureFilterWithDirectXTK::CKDX_TextureFilterWithDirectXTK(HINSTANCE hInstance)
	: D3DApp(hInstance), mBoxVB(0), mBoxIB(0), mDiffuseMapSRV(0), mEyePosW(0.0f, 0.0f, 0.0f),
	mTheta(1.3f * MathHelper::Pi), mPhi(0.4f * MathHelper::Pi), mRadius(2.5f)
{
	mMainWndCaption = L"TextureFilte Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mBoxWorld, I);
	XMStoreFloat4x4(&mTexTransform, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);

	mBoxMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);

	mDirLights[0].Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mDirLights[0].Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	mDirLights[0].Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
	mDirLights[0].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);
	//mDirLights[0].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	//mDirLights[0].Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	//mDirLights[0].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	//mDirLights[0].Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);

	mDirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	mDirLights[1].Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	mDirLights[1].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	mDirLights[1].Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);

	mDirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	mDirLights[2].Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	mDirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	mDirLights[2].Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

CKDX_TextureFilterWithDirectXTK::~CKDX_TextureFilterWithDirectXTK()
{
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);
	ReleaseCOM(mDiffuseMapSRV);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
}

bool CKDX_TextureFilterWithDirectXTK::Init()
{
	if (!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);

	// https://github.com/walbourn/directx-sdk-samples 에서 dds 파일 로더 부분 가지고 옴
	HR(CreateWICTextureFromFile(md3dDevice, L"Resources/WoodCrateJPG.jpg", nullptr, &mDiffuseMapSRV));
	
	BuildGeometryBuffers();

	return true;
}

void CKDX_TextureFilterWithDirectXTK::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void CKDX_TextureFilterWithDirectXTK::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	mEyePosW = XMFLOAT3(x, y, z);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);
}

void CKDX_TextureFilterWithDirectXTK::DrawScene()
{
	// 화면 초기화, 깊이와 스텐실 버퍼 초기화
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// 정점들을 삼각형으로 렌더링한다는 코드
	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	// 카메라 뷰 행렬, 투영 행렬, 월드 좌표 -> 화면 좌표
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view * proj;

	// Set per frame constants.	
	Effects::BasicFX->SetEyePosW(mEyePosW);

	Effects::BasicFX->SetDirLights(mDirLights);

	ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light0TexTech;
	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		// 정점 버퍼와 인덱스 버퍼를 설정.
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

		// 객체를 월드 공간로 변환
		XMMATRIX world = XMLoadFloat4x4(&mBoxWorld);
		// 조명 계산을 위한 월드 행렬의 역전치 행렬. 정점의 법선을 변환할 때 사용.
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		// 월드 좌표 -> 클립 공간으로 변환할 때 사용하는 변환 행렬
		XMMATRIX worldViewProj = world * view * proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mTexTransform)); // 텍스처 변환
		Effects::BasicFX->SetMaterial(mBoxMat); // 머티리얼 
		Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRV); // 텍스처

		// 셰이더를 적용, 설정된 인덱스 버퍼에 따라 정육면체를 그림.
		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);
	}

	// 스왑체인을 통한 화면에 그리기.
	HR(mSwapChain->Present(0, 0));
}

void CKDX_TextureFilterWithDirectXTK::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void CKDX_TextureFilterWithDirectXTK::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CKDX_TextureFilterWithDirectXTK::OnMouseMove(WPARAM btnState, int x, int y)
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

void CKDX_TextureFilterWithDirectXTK::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData box;

	GeometryGenerator geoGen;

	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
	// geoGen.CreateSphere(1.0f, 1024, 1024, box);  // slice : 가로 방향, stack :세로방향으로 몇등분할 것인지 나타내는것 
	// geoGen.CreateGrid(10.0f, 10.0f, 512, 512, box);  // slice : 가로 방향, stack :세로방향으로 몇등분할 것인지 나타내는것 

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	mBoxVertexOffset = 0;

	// Cache the index count of each object.
	mBoxIndexCount = (UINT)box.Indices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	mBoxIndexOffset = 0;

	UINT totalVertexCount = (UINT)box.Vertices.size();

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

	D3D11_SAMPLER_DESC sampleDesc;
	ZeroMemory(&sampleDesc, sizeof(sampleDesc));
	sampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	sampleDesc.MaxAnisotropy = 8;
	sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;


	ID3D11SamplerState* samplerState = nullptr;
	md3dDevice->CreateSamplerState(&sampleDesc, &samplerState);
	md3dImmediateContext->PSSetSamplers(0, 1, &samplerState);
}

