// Effect_Direct3D_Lighting_00_RenderSphere.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "Effect_Direct3D_Lighting_00_RenderSphere.h"

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

	CKDX_LightingGeo theApp(hInstance);

	/*char* cp;
	cp = (char*)malloc(16);*/

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

CKDX_LightingGeo::CKDX_LightingGeo(HINSTANCE hInstance)
	: D3DApp(hInstance), mGeoVB(0), mGeoIB(0), 
	mInputLayout(0), mEyePosW(0.0f, 0.0f, 0.0f), mTheta(1.5f * MathHelper::Pi), mPhi(0.1f * MathHelper::Pi), mRadius(80.0f)
{
	mMainWndCaption = L"Crate Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mGeoWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
	
	mDirLights.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	mDirLights.Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights.Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	mDirLights.Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);


	// Point light--position is changed every frame to animate in UpdateScene function.
	mPointLight.Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	mPointLight.Diffuse = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);	
	mPointLight.Specular = XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
	mPointLight.Att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	mPointLight.Range = 25.0f;

	// Spot light--position and direction changed every frame to animate in UpdateScene function.
	mSpotLight.Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	mSpotLight.Diffuse = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	mSpotLight.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mSpotLight.Att = XMFLOAT3(1.0f, 0.0f, 0.0f);
	mSpotLight.Spot = 96.0f;
	mSpotLight.Range = 10000.0f;

	mGeoMat.Ambient = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	mGeoMat.Diffuse = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
	mGeoMat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
}

CKDX_LightingGeo::~CKDX_LightingGeo()
{
	ReleaseCOM(mGeoVB);
	ReleaseCOM(mGeoIB);
	ReleaseCOM(mDiffuseMapSRV);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
}

bool CKDX_LightingGeo::Init()
{
	if (!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	BuildGeometryBuffers();
	BuildFX();
	BuildVertexLayout();
	return true;
}

void CKDX_LightingGeo::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void CKDX_LightingGeo::UpdateScene(float dt)
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

	// Circle light over the land surface.
	mPointLight.Position.x = 70.0f * cosf(0.2f * mTimer.TotalTime());
	mPointLight.Position.z = 70.0f * sinf(0.2f * mTimer.TotalTime());
	mPointLight.Position.y = 10.0f;	

	// The spotlight takes on the camera position and is aimed in the
	// same direction the camera is looking.  In this way, it looks
	// like we are holding a flashlight.
	mSpotLight.Position = mEyePosW;
	XMStoreFloat3(&mSpotLight.Direction, XMVector3Normalize(target - pos));
}

void CKDX_LightingGeo::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);	
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex::Basic32);	
	UINT offset = 0;

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view * proj;

	Effects::BasicFX->SetDirLights(mDirLights);
	Effects::BasicFX->SetPointLight(mPointLight);
	Effects::BasicFX->SetSpotLight(mSpotLight);
	Effects::BasicFX->SetEyePosW(mEyePosW);	

	ID3DX11EffectTechnique* activeTech = Effects::BasicFX->LightTech;
	D3DX11_TECHNIQUE_DESC techDesc;
	activeTech->GetDesc(&techDesc);

	for (UINT p = 0; p < techDesc.Passes; ++p)
	{	
		md3dImmediateContext->IASetVertexBuffers(0, 1, &mGeoVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mGeoIB, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&mGeoWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);		
		Effects::BasicFX->SetMaterial(mGeoMat);

		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mGeoIndexCount, 0, 0);
	}

	HR(mSwapChain->Present(0, 0));
}

void CKDX_LightingGeo::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void CKDX_LightingGeo::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CKDX_LightingGeo::OnMouseMove(WPARAM btnState, int x, int y)
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
		// Make each pixel correspond to 0.2 unit in the scene.
		float dx = 0.2f * static_cast<float>(x - mLastMousePos.x);
		float dy = 0.2f * static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 50.0f, 500.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void CKDX_LightingGeo::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData geo;

	GeometryGenerator geoGen;
	geoGen.CreateBox(10.0f, 10.0f, 10.0f, geo);
	//geoGen.CreateSphere(1.0f, 16, 16, geo);  // slice : 가로 방향, stack :세로방향으로 몇등분할 것인지 나타내는것 

	// geoGen.CreateGrid(160.0f, 160.0f, 50, 50, geo);

	// Cache the vertex offsets to each object in the concatenated vertex buffer.
	mGeoVertexOffset = 0;

	// Cache the index count of each object.
	mGeoIndexCount = (UINT)geo.Indices.size();

	// Cache the starting index for each object in the concatenated index buffer.
	mGeoIndexOffset = 0;

	UINT totalVertexCount = (UINT)geo.Vertices.size();

	UINT totalIndexCount = mGeoIndexCount;

	//
	// Extract the vertex elements we are interested in and pack the
	// vertices of all the meshes into one vertex buffer.
	//

	std::vector<Vertex::Basic32> vertices(totalVertexCount);

	UINT k = 0;
	for (size_t i = 0; i < geo.Vertices.size(); ++i, ++k)
	{
		vertices[k].Pos = geo.Vertices[i].Position;
		vertices[k].Normal = geo.Vertices[i].Normal;		
	}

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(Vertex::Basic32) * totalVertexCount;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &vertices[0];
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mGeoVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//
	std::vector<UINT> indices;
	indices.insert(indices.end(), geo.Indices.begin(), geo.Indices.end());

	D3D11_BUFFER_DESC ibd;
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(UINT) * totalIndexCount;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = &indices[0];
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mGeoIB));
}

void CKDX_LightingGeo::BuildFX()
{
	Effects::InitAll(md3dDevice);
}

void CKDX_LightingGeo::BuildVertexLayout()
{
	InputLayouts::InitAll(md3dDevice);
}