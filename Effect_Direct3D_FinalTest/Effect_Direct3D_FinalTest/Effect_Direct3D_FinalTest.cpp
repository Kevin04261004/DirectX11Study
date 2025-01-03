﻿// Effect_Direct3D_FinalTest.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "Effect_Direct3D_FinalTest.h"

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

	CK_DX_ComputeTexFilter theApp(hInstance);

	/*char* cp;
	cp = (char*)malloc(16);*/

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}



CK_DX_ComputeTexFilter::CK_DX_ComputeTexFilter(HINSTANCE hInstance)
	: D3DApp(hInstance), mBoxVB(0), mBoxIB(0), mSrcTextureData(0), mSrcTexture(0), mTextureDataSize(0),
	mInputASRV(0), mOutputBuffer(0), mOutputUAV(0), mDestTexture(0), mDestTextureView(0),
	mTextureSRV(0), mEyePosW(0.0f, 0.0f, 0.0f), mTheta(-0.5f * MathHelper::Pi), mPhi(0.5f * MathHelper::Pi), mRadius(3.0f)
{
	mMainWndCaption = L"CK_DX_FinalTest";
	mEnable4xMsaa = false;
	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
	XMStoreFloat4x4(&mBoxWorld, I);
	mBoxPosition = I;
	mPlanePosition = XMMatrixTranslation(0, -1, 0);

	mBoxMat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mBoxMat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mBoxMat.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
}

CK_DX_ComputeTexFilter::~CK_DX_ComputeTexFilter()
{
	md3dImmediateContext->ClearState();

	SafeDelete(mSrcTextureData);
	ReleaseCOM(mSrcTexture);
	ReleaseCOM(mInputASRV);
	ReleaseCOM(mOutputBuffer);
	ReleaseCOM(mOutputUAV);
	ReleaseCOM(mDestTexture);
	ReleaseCOM(mDestTextureView);

	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);

	ReleaseCOM(mTextureSRV);


	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool CK_DX_ComputeTexFilter::Init()
{
	if (!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);
	RenderStates::InitAll(md3dDevice);

	BuildInBuffer();
	BuildOutBuffer();
	BuildCrateGeometryBuffers();

	return true;
}

void CK_DX_ComputeTexFilter::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void CK_DX_ComputeTexFilter::UpdateScene(float dt)
{
	// 이동 속도와 회전 속도
	float moveSpeed = 5.0f * dt; // 초당 10단위 이동
	float rotateSpeed = XMConvertToRadians(90.0f) * dt; // 초당 45도 회전

	// 사용자 입력 처리 (키보드 입력 예제, 실제 입력 처리는 다른 방식으로 구현될 수 있음)
	if (GetAsyncKeyState(VK_UP) & 0x8000) // 위 화살표
	{
		// 자동차가 바라보는 방향(전방 벡터)을 따라 이동
		XMVECTOR forward = XMVectorSet(mBoxPosition.r[2].m128_f32[0],  // Z축 방향 벡터의 X
			mBoxPosition.r[2].m128_f32[1],  // Z축 방향 벡터의 Y
			mBoxPosition.r[2].m128_f32[2],  // Z축 방향 벡터의 Z
			0.0f);                          // W = 0 (벡터)
		XMVECTOR translation = forward * moveSpeed;
		mBoxPosition.r[3] += translation; // 평행 이동
	}

	if (GetAsyncKeyState(VK_DOWN) & 0x8000) // 아래 화살표
	{
		// 뒤로 이동 (전방 벡터의 반대 방향)
		XMVECTOR forward = XMVectorSet(mBoxPosition.r[2].m128_f32[0],
			mBoxPosition.r[2].m128_f32[1],
			mBoxPosition.r[2].m128_f32[2],
			0.0f);
		XMVECTOR translation = -forward * moveSpeed;
		mBoxPosition.r[3] += translation; // 평행 이동
	}

	if (GetAsyncKeyState(VK_LEFT) & 0x8000) // 왼쪽 화살표
	{
		// 자동차의 Y축을 기준으로 반시계 방향 회전
		XMMATRIX rotation = XMMatrixRotationY(-rotateSpeed);
		mBoxPosition = XMMatrixMultiply(rotation, mBoxPosition);
	}

	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) // 오른쪽 화살표
	{
		// 자동차의 Y축을 기준으로 시계 방향 회전
		XMMATRIX rotation = XMMatrixRotationY(rotateSpeed);
		mBoxPosition = XMMatrixMultiply(rotation, mBoxPosition);
	}

	if (GetAsyncKeyState('1') & 0x8000) // 1
	{
		md3dImmediateContext->RSSetState(0);
		md3dImmediateContext->RSSetState(RenderStates::CullRS);
	}

	if (GetAsyncKeyState('2') & 0x8000) // 2
	{
		md3dImmediateContext->RSSetState(0);
		md3dImmediateContext->RSSetState(RenderStates::NoCullRS);
	}

	if (GetAsyncKeyState('3') & 0x8000) // 3
	{
		md3dImmediateContext->RSSetState(0);
		md3dImmediateContext->RSSetState(RenderStates::WireframeRS);
	}

	if (GetAsyncKeyState('4') & 0x8000) // 4
	{
		md3dImmediateContext->RSSetState(0);
		md3dImmediateContext->RSSetState(RenderStates::NoCullWireframeRS);
	}


	// 키가 현재 눌려있지 않은 상태 확인 (상위 비트가 0)
	SHORT state = GetAsyncKeyState('Q');

	bool isKeyReleasedNow = (state & 0x8000) == 0;
	if (mQKeyPressed && isKeyReleasedNow) {
		mUseCS = !mUseCS;
		if (mUseCS) DoComputeWork();
	}
	// 현재 키의 상태 업데이트
	mQKeyPressed = (state & 0x8000) != 0;

	UpdateCamera(dt);
}

void CK_DX_ComputeTexFilter::UpdateCamera(float dt)
{
	// 카메라 위치 계산 (원래 코드 유지)
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	XMFLOAT3 boxPos;
	XMStoreFloat3(&boxPos, mBoxPosition.r[3]);

	// 뷰 행렬 생성
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero(); // XMVectorSet(boxPos.x, boxPos.y, boxPos.z, 1.0f);
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX V = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, V);
}

void CK_DX_ComputeTexFilter::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	XMMATRIX finalM, transM, scalM;
	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view * proj;

	// Set per frame constants.
	ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light0TexTech;
	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);

	XMMATRIX parentM;
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		/* Plane */
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mPlaneVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mPlaneIB, DXGI_FORMAT_R32_UINT, 0);
		
		scalM = XMMatrixScaling(10.0f, 10.0f, 10.0f);
		transM = mPlanePosition;
		finalM = scalM * transM;
		
		XMStoreFloat4x4(&mBoxWorld, finalM);
		
		XMMATRIX world = XMLoadFloat4x4(&mBoxWorld);
		XMMATRIX worldViewProj = world * view * proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(mBoxMat);
		Effects::BasicFX->SetDiffuseMap(mThirdTextureSRV);

		// 해당 pass에 d3d context의 설정을 적용 
		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

		// indexed로 렌더링 
		md3dImmediateContext->DrawIndexed(boxIndexSize, 0, offset);

		/* BOX */
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);
		scalM = XMMatrixScaling(1.0f, 1.0f, 2.0f);
		transM = mBoxPosition;
		XMMATRIX parentMatrix = transM;
		finalM = scalM * transM;
		XMStoreFloat4x4(&mBoxWorld, finalM);
		world = XMLoadFloat4x4(&mBoxWorld);
		worldViewProj = world * view * proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(mBoxMat);

		if (!mUseCS)
		{
			Effects::BasicFX->SetDiffuseMap(mTextureSRV);
		}
		else
		{
			Effects::BasicFX->SetDiffuseMap(mDestTextureView);
		}

		// 해당 pass에 d3d context의 설정을 적용 
		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

		// indexed로 렌더링 
		md3dImmediateContext->DrawIndexed(boxIndexSize, 0, offset);

		XMMATRIX mt[4];
		mt[0] = XMMatrixTranslation(0.375f, -0.75f, -0.75f);
		mt[1] = XMMatrixTranslation(-0.375f, -0.75f, -0.75f);
		mt[2] = XMMatrixTranslation(0.375f, -0.75f, 0.75f);
		mt[3] = XMMatrixTranslation(-0.375f, -0.75f, 0.75f);
		XMMATRIX rotation = XMMatrixRotationX(XMConvertToRadians(90.0f));
		rotation = rotation * XMMatrixRotationY(XMConvertToRadians(90.0f));

		/* Wheel (Cylinder) */
		for (int i = 0; i < 4; ++i)
		{
			md3dImmediateContext->IASetVertexBuffers(0, 1, &mCylinderVB, &stride, &offset);
			md3dImmediateContext->IASetIndexBuffer(mCylinderIB, DXGI_FORMAT_R32_UINT, 0);
			scalM = XMMatrixScaling(0.25f, 0.25f, 0.25f);
			transM = mt[i] * parentMatrix;
			finalM = scalM * rotation * transM;
			XMStoreFloat4x4(&mBoxWorld, finalM);

			// Draw the box. 
			world = XMLoadFloat4x4(&mBoxWorld);
			worldViewProj = world * view * proj;
			Effects::BasicFX->SetWorld(world);
			Effects::BasicFX->SetWorldViewProj(worldViewProj);
			Effects::BasicFX->SetDiffuseMap(mSecondTextureSRV);

			// 해당 pass에 d3d context의 설정을 적용
			activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
			md3dImmediateContext->DrawIndexed(cylinderIndexSize, 0, 0);
		}
	}

	/* Draw Normal Line */
	activeTech = Effects::BasicFX->VisibleNormalTech;
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		/* BOX */
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);
		scalM = XMMatrixScaling(1.0f, 1.0f, 2.0f);
		transM = mBoxPosition;
		XMMATRIX parentMatrix = transM;
		finalM = scalM * transM;
		XMStoreFloat4x4(&mBoxWorld, finalM);
		XMMATRIX world = XMLoadFloat4x4(&mBoxWorld);
		XMMATRIX worldViewProj = world * view * proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(mBoxMat);
		Effects::BasicFX->SetDiffuseMap(mTextureSRV);

		// 해당 pass에 d3d context의 설정을 적용 
		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

		// indexed로 렌더링 
		md3dImmediateContext->DrawIndexed(boxIndexSize, 0, offset);
	}

	HR(mSwapChain->Present(0, 0));
}

void CK_DX_ComputeTexFilter::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;
	SetCapture(mhMainWnd);
}

void CK_DX_ComputeTexFilter::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CK_DX_ComputeTexFilter::OnMouseMove(WPARAM btnState, int x, int y)
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

void CK_DX_ComputeTexFilter::BuildCrateGeometryBuffers()
{
	GeometryGenerator::MeshData box;
	GeometryGenerator::MeshData plane;
	GeometryGenerator::MeshData cylinder;
	GeometryGenerator geoGen;

	geoGen.CreateBox(1, 1, 1, box);
	boxIndexSize = box.Indices.size();
	boxVertexSize = box.Vertices.size();

	geoGen.CreateGrid(10, 10, 4, 4, plane);
	planeIndexSize = plane.Indices.size();
	planeVertexSize = plane.Vertices.size();

	geoGen.CreateCylinder(1, 1, 1, 30, 1, cylinder);
	cylinderIndexSize = cylinder.Indices.size();
	cylinderVertexSize = cylinder.Vertices.size();

	std::vector<GeometryGenerator::MeshData> meshDatas;
	meshDatas.push_back(box);
	meshDatas.push_back(plane);
	meshDatas.push_back(cylinder);

	std::vector<ID3D11Buffer**> meshVertexBuffers;
	meshVertexBuffers.push_back(&mBoxVB);
	meshVertexBuffers.push_back(&mPlaneVB);
	meshVertexBuffers.push_back(&mCylinderVB);

	std::vector<ID3D11Buffer**> meshIndexBuffers;
	meshIndexBuffers.push_back(&mBoxIB);
	meshIndexBuffers.push_back(&mPlaneIB);
	meshIndexBuffers.push_back(&mCylinderIB);

	assert(meshDatas.size() == meshVertexBuffers.size() && meshDatas.size() == meshIndexBuffers.size());

	for (int i = 0; i < meshDatas.size(); ++i)
	{
		std::vector<Vertex::Basic32> vertices(meshDatas[i].Vertices.size());

		for (UINT j = 0; j < meshDatas[i].Vertices.size(); ++j)
		{
			vertices[j].Pos = meshDatas[i].Vertices[j].Position;
			vertices[j].Normal = meshDatas[i].Vertices[j].Normal;
			vertices[j].Tex = meshDatas[i].Vertices[j].TexC;
		}

		D3D11_BUFFER_DESC vbd;
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(Vertex::Basic32) * meshDatas[i].Vertices.size();
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA vinitData;
		vinitData.pSysMem = &vertices[0];
		HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &*meshVertexBuffers[i]));

		D3D11_BUFFER_DESC ibd;
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = sizeof(UINT) * meshDatas[i].Indices.size();
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA iinitData;
		iinitData.pSysMem = &meshDatas[i].Indices[0];
		HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &*meshIndexBuffers[i]));
	}
	
	//D3D11_SAMPLER_DESC sampleDesc;
	//ZeroMemory(&sampleDesc, sizeof(sampleDesc));
	//sampleDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	//sampleDesc.MaxAnisotropy = 8;
	//sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	//sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;

	//ID3D11SamplerState* samplerState = nullptr;
	//md3dDevice->CreateSamplerState(&sampleDesc, &samplerState);
	//md3dImmediateContext->PSSetSamplers(0, 1, &samplerState);
}

void CK_DX_ComputeTexFilter::BuildInBuffer()
{
	//=============================================================================
	// 0. Texture를 읽어 들이고 Texture Data를 얻음 
	//=============================================================================
	ID3D11Texture2D* tempTexture;

	// 해당 Texture를 우선 읽어 들인다.		
	HR(CreateDDSTextureFromFile(md3dDevice, L"Textures/brick01.dds", (ID3D11Resource**)(&mSrcTexture), &mSecondTextureSRV));
	HR(CreateDDSTextureFromFile(md3dDevice, L"Textures/grass.dds", (ID3D11Resource**)(&mSrcTexture), &mThirdTextureSRV));
	HR(CreateDDSTextureFromFile(md3dDevice, L"Textures/amazon.dds", (ID3D11Resource**)(&mSrcTexture), &mTextureSRV));

	D3D11_TEXTURE2D_DESC desc;
	(mSrcTexture)->GetDesc(&desc);

	// 32비트 텍스처 처리를 위함 그렇지 않은 데이터들은 실패  
	if (desc.Format != DXGI_FORMAT_R8G8B8A8_UNORM) {
		return;
	}

	// D3D11_USAGE_STAGING : GPU에서 CPU로의 데이터 전송(복사)을 지원하는 리소스입니다.
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	if (md3dDevice->CreateTexture2D(&desc, NULL, &tempTexture) != S_OK)
		return;

	// Src 리소스 내용을 Dest 리소스에 복사한다. 
	md3dImmediateContext->CopyResource(tempTexture, mSrcTexture);

	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Map 메서드를 통해서 ID3D11Texture2D을 UnLock
	if (md3dImmediateContext->Map(tempTexture, 0, D3D11_MAP_READ, 0, &mappedResource) != S_OK)
		return;

	mTextureDataSize = mappedResource.RowPitch * desc.Height;

	// 기존 데이터 삭제 
	if (mSrcTextureData) {
		delete[] mSrcTextureData;
		mSrcTextureData = NULL;
	}

	mSrcTextureData = new byte[mTextureDataSize];
	memcpy(mSrcTextureData, mappedResource.pData, mTextureDataSize);

	md3dImmediateContext->Unmap(tempTexture, 0);

	ReleaseCOM(tempTexture);

	//=============================================================================
	// 1. Texture에서 읽은 데이터를 가지고 와서 Input Buffer를 처리해야 한다. (BufferA 생성)
	// Create a buffer to be bound as a shader input (D3D11_BIND_SHADER_RESOURCE).
	//=============================================================================
	D3D11_BUFFER_DESC inputDesc;
	ZeroMemory(&inputDesc, sizeof(inputDesc));
	inputDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	inputDesc.Usage = D3D11_USAGE_DEFAULT;
	inputDesc.ByteWidth = mTextureDataSize;
	inputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	inputDesc.StructureByteStride = 4; // sizeof(structure) : RGBA 

	D3D11_SUBRESOURCE_DATA vinitDataA;
	vinitDataA.pSysMem = mSrcTextureData;

	// bufferA가 GPU에 Data를 넘겨줄 Buffer Data임 , Compute Shader에게 입력버퍼
	ID3D11Buffer* bufferA = 0;
	HR(md3dDevice->CreateBuffer(&inputDesc, &vinitDataA, &bufferA));

	//=======================================================
	// 2. bufferA에 대한 ShaderResourceView생성 
	//=======================================================
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	srvDesc.BufferEx.FirstElement = 0;
	srvDesc.BufferEx.Flags = 0;
	// NumElements : 512*512
	srvDesc.BufferEx.NumElements = mTextureDataSize / inputDesc.StructureByteStride;

	md3dDevice->CreateShaderResourceView(bufferA, &srvDesc, &mInputASRV);
}

void CK_DX_ComputeTexFilter::BuildOutBuffer()
{
	// Create a read-write buffer the compute shader can write to (D3D11_BIND_UNORDERED_ACCESS).
	// 1. compute shader가 읽기/쓰기 가능한 outputbuffer 생성
	D3D11_BUFFER_DESC outputDesc;
	ZeroMemory(&outputDesc, sizeof(outputDesc));
	outputDesc.Usage = D3D11_USAGE_DEFAULT;
	outputDesc.ByteWidth = mTextureDataSize;
	outputDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	outputDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	outputDesc.StructureByteStride = 4;

	md3dDevice->CreateBuffer(&outputDesc, 0, &mOutputBuffer);

	// 2. bufferA에 대한 ShaderResourceView생성 
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(uavDesc));

	// Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = mTextureDataSize / outputDesc.StructureByteStride;

	md3dDevice->CreateUnorderedAccessView(mOutputBuffer, &uavDesc, &mOutputUAV);
}

void CK_DX_ComputeTexFilter::DoComputeWork()
{
	D3DX11_TECHNIQUE_DESC techDesc;
	Effects::TexturedFX->SetInputA(mInputASRV);
	Effects::TexturedFX->SetOutput(mOutputUAV);
	Effects::TexturedFX->TextureEffectTech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = Effects::TexturedFX->TextureEffectTech->GetPassByIndex(p);
		pass->Apply(0, md3dImmediateContext);
		md3dImmediateContext->Dispatch(16, 16, 1);
	}

	// compute shader로 부터 input textures 접근 해제 처리 
	ID3D11ShaderResourceView* nullSRV[1] = { 0 };
	md3dImmediateContext->CSSetShaderResources(0, 1, nullSRV);

	// compute shader로 부터 output 접근 해제 처리 
	ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
	md3dImmediateContext->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	// compute shader 불가능 처리 
	md3dImmediateContext->CSSetShader(0, 0, 0);

	// 결과값을 mDestTexture에 저장 진행 
	if (mDestTexture)
		mDestTexture->Release();

	// 동적으로 텍스처 생성 
	D3D11_TEXTURE2D_DESC desc;
	mSrcTexture->GetDesc(&desc);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.MipLevels = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	HR(md3dDevice->CreateTexture2D(&desc, NULL, &mDestTexture))

		D3D11_MAPPED_SUBRESOURCE mappedResource;
	md3dImmediateContext->Map(mDestTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// GPU의 계산 내용을 outBuff에서 받음, Texture를 만들어서 처리 필요 
	byte* outBuff = NULL;

	ID3D11Buffer* debugbuf = NULL;
	D3D11_BUFFER_DESC outputdesc;
	ZeroMemory(&outputdesc, sizeof(outputdesc));
	mOutputBuffer->GetDesc(&outputdesc);

	UINT byteSize = outputdesc.ByteWidth;
	outputdesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	outputdesc.Usage = D3D11_USAGE_STAGING;
	outputdesc.BindFlags = 0;
	outputdesc.MiscFlags = 0;

	if (SUCCEEDED(md3dDevice->CreateBuffer(&outputdesc, NULL, &debugbuf)))
	{
		// mOutputBuffer의 내용을 debugbuf로 이동 
		md3dImmediateContext->CopyResource(debugbuf, mOutputBuffer);

		D3D11_MAPPED_SUBRESOURCE mappedSubResource;
		if (md3dImmediateContext->Map(debugbuf, 0, D3D11_MAP_READ, 0, &mappedSubResource) != S_OK)
			return;

		outBuff = new byte[byteSize];
		memcpy(outBuff, mappedSubResource.pData, byteSize);

		md3dImmediateContext->Unmap(debugbuf, 0);

		debugbuf->Release();
	}


	memcpy(mappedResource.pData, outBuff, mTextureDataSize);

	md3dImmediateContext->Unmap(mDestTexture, 0);

	// Create a view of the output texture
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	ZeroMemory(&viewDesc, sizeof(viewDesc));
	viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	viewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
	viewDesc.Texture2D.MipLevels = 1;
	viewDesc.Texture2D.MostDetailedMip = 0;

	md3dDevice->CreateShaderResourceView(mDestTexture, &viewDesc, &mDestTextureView);
}