// Effect_Direct3D_BlendStencil_05_StencilTexBox.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "Effect_Direct3D_BlendStencil_05_StencilTexBox.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{

	// 디버그 빌드 할때 CRT를 이용한 런타임 메모리릭 찾기를 수행
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	CK_DX_StencilTextureBox theApp(hInstance);

	/*char* cp;
	cp = (char*)malloc(16);*/

	if (!theApp.Init())
		return 0;

	return theApp.Run();
}

CK_DX_StencilTextureBox::CK_DX_StencilTextureBox(HINSTANCE hInstance)
	: D3DApp(hInstance), mBoxVB(0), mBoxIB(0), mDiffuseMapSRV(0), mGridDiffuseMapSRV(0), mEyePosW(0.0f, 0.0f, 0.0f),
	mTheta(1.3f * MathHelper::Pi), mPhi(0.4f * MathHelper::Pi), mRadius(5.0f)
{
	mMainWndCaption = L"CKDK_Stencil TextureBox Demo";

	mLastMousePos.x = 0;
	mLastMousePos.y = 0;

	XMMATRIX I = XMMatrixIdentity();
	XMStoreFloat4x4(&mBoxWorld, I);
	XMStoreFloat4x4(&mGridWorld, I);
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

CK_DX_StencilTextureBox::~CK_DX_StencilTextureBox()
{
	ReleaseCOM(mBoxVB);
	ReleaseCOM(mBoxIB);

	ReleaseCOM(mGrassGridQuadVB);
	ReleaseCOM(mGrassGridQuadVB);

	ReleaseCOM(mGridQuadVB);
	ReleaseCOM(mGridQuadIB);

	ReleaseCOM(mDiffuseMapSRV);
	ReleaseCOM(mGridDiffuseMapSRV);
	ReleaseCOM(mGrassGridDiffuseMapSRV);

	Effects::DestroyAll();
	InputLayouts::DestroyAll();
	RenderStates::DestroyAll();
}

bool CK_DX_StencilTextureBox::Init()
{
	if (!D3DApp::Init())
		return false;

	// Must init Effects first since InputLayouts depend on shader signatures.
	Effects::InitAll(md3dDevice);
	InputLayouts::InitAll(md3dDevice);
	RenderStates::InitAll(md3dDevice);

	// https://github.com/walbourn/directx-sdk-samples 에서 dds 파일 로더 부분 가지고 옴
	//HR(CreateDDSTextureFromFile(md3dDevice, L"Textures/WireFence.dds", nullptr, &mDiffuseMapSRV));
	HR(CreateDDSTextureFromFile(md3dDevice, L"Textures/WoodCrate01.dds", nullptr, &mDiffuseMapSRV));
	//HR(CreateDDSTextureFromFile(md3dDevice, L"Textures/water2.dds", nullptr, &mDiffuseMapSRV));

	HR(CreateDDSTextureFromFile(md3dDevice, L"Textures/red.dds", nullptr, &mGridDiffuseMapSRV));
	HR(CreateDDSTextureFromFile(md3dDevice, L"Textures/grass.dds", nullptr, &mGrassGridDiffuseMapSRV));

	BuildGeometryBuffers();

	return true;
}

void CK_DX_StencilTextureBox::OnResize()
{
	D3DApp::OnResize();

	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void CK_DX_StencilTextureBox::UpdateScene(float dt)
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

	// Init the grid world matrix.
	XMMATRIX gridRotate = XMMatrixRotationX(-0.5f * MathHelper::Pi);
	XMMATRIX gridScale = XMMatrixScaling(1.f, 1.f, 1.f);
	XMMATRIX gridTrans = XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	XMStoreFloat4x4(&mGridWorld, gridScale * gridRotate * gridTrans);

	// Init the grass grid world matrix.
	XMMATRIX grassGridRotate = XMMatrixRotationX(-0.5f * MathHelper::Pi);
	XMMATRIX grassGridScale = XMMatrixScaling(1.f, 1.f, 1.f);
	XMMATRIX grassGridTrans = XMMatrixTranslation(0.0f, 0.0f, 0.1f);
	XMStoreFloat4x4(&mGrassGridWorld, grassGridScale * grassGridRotate * grassGridTrans);

	// Init the new world matrix.
	XMMATRIX boxRotate = XMMatrixRotationX(0.f * MathHelper::Pi);
	XMMATRIX boxScale = XMMatrixScaling(1.f, 1.f, 1.f);
	XMMATRIX boxTrans = XMMatrixTranslation(0.0f, 0.0f, 2.0f);
	XMStoreFloat4x4(&mBoxWorld, boxScale * boxRotate * boxTrans);


}

void CK_DX_StencilTextureBox::DrawScene()
{
	md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
	md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dImmediateContext->IASetInputLayout(InputLayouts::Basic32);
	md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(Vertex::Basic32);
	UINT offset = 0;
	//float blendFactor[] = { 0.3f, 0.3f, 0.3f, 1.0f };
	//float blendFactor[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	float blendFactor[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);
	XMMATRIX viewProj = view * proj;

	// Set per frame constants.
	Effects::BasicFX->SetDirLights(mDirLights);
	Effects::BasicFX->SetEyePosW(mEyePosW);

	// Light1과 Light2에 대한 부분과 texture까지 처리하기 위해서 Light2TexTech 사용함 
	//ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light2TexAlphaClipTech;
	ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light2TexTech;
	//ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light0TexTech;
	//ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light1TexTech;
	//ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light1Tech;
	//ID3DX11EffectTechnique* activeTech = Effects::BasicFX->Light2Tech;


	D3DX11_TECHNIQUE_DESC techDesc;
#if 0
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

		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

		// BLEND_TEXTURE alpha clipping : WireFence.dds : No Transparent , Light2TexAlphaClipTech		
		md3dImmediateContext->RSSetState(RenderStates::NoCullRS);
		md3dImmediateContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);

		// Restore default render state.
		md3dImmediateContext->RSSetState(0);

		// Restore default blend state
		md3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);
		md3dImmediateContext->OMSetDepthStencilState(0, 0);
	}
	//// draw grass grid 
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

		md3dImmediateContext->IASetVertexBuffers(0, 1, &mGrassGridQuadVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mGrassGridQuadIB, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&mGrassGridWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(mBoxMat);
		Effects::BasicFX->SetDiffuseMap(mGrassGridDiffuseMapSRV);

		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

		md3dImmediateContext->DrawIndexed(mGrassGridIndexCount, mGrassGridIndexOffset, mGrassGridVertexOffset);
	}

	// draw grid 
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

		md3dImmediateContext->IASetVertexBuffers(0, 1, &mGridQuadVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mGridQuadIB, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&mGridWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(mBoxMat);
		Effects::BasicFX->SetDiffuseMap(mGridDiffuseMapSRV);

		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

		md3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);
	}
#endif


#if 1 
	//
	// Draw the grid to stencil buffer only.
	//
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

		md3dImmediateContext->IASetVertexBuffers(0, 1, &mGridQuadVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mGridQuadIB, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&mGridWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());

		// Do not write to render target.
		md3dImmediateContext->OMSetBlendState(RenderStates::NoRenderTargetWritesBS, blendFactor, 0xffffffff);

		// Render visible pixels to stencil buffer.
		// Do not write depth to depth buffer at this point
		md3dImmediateContext->OMSetDepthStencilState(RenderStates::MarkDSS, 1);

		pass->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

		// Restore states.
		md3dImmediateContext->OMSetDepthStencilState(0, 0);
		md3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);
	}

	//
	// Draw the box 
	//
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

		md3dImmediateContext->IASetVertexBuffers(0, 1, &mBoxVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mBoxIB, DXGI_FORMAT_R32_UINT, 0);

		XMMATRIX world = XMLoadFloat4x4(&mBoxWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMLoadFloat4x4(&mTexTransform));
		Effects::BasicFX->SetMaterial(mBoxMat);
		Effects::BasicFX->SetDiffuseMap(mDiffuseMapSRV);

		// Only draw into visible pixels as marked by the stencil buffer. 		
		md3dImmediateContext->OMSetDepthStencilState(RenderStates::DrawDSS, 1);
		pass->Apply(0, md3dImmediateContext);
		md3dImmediateContext->DrawIndexed(mBoxIndexCount, 0, 0);

		// Restore default states.
		md3dImmediateContext->RSSetState(0);
		md3dImmediateContext->OMSetDepthStencilState(0, 0);
	}

	// draw grid 	
	activeTech->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

		md3dImmediateContext->IASetVertexBuffers(0, 1, &mGridQuadVB, &stride, &offset);
		md3dImmediateContext->IASetIndexBuffer(mGridQuadIB, DXGI_FORMAT_R32_UINT, 0);

		// Set per object constants.
		XMMATRIX world = XMLoadFloat4x4(&mGridWorld);
		XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
		XMMATRIX worldViewProj = world * view * proj;

		Effects::BasicFX->SetWorld(world);
		Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
		Effects::BasicFX->SetWorldViewProj(worldViewProj);
		Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
		Effects::BasicFX->SetMaterial(mBoxMat);

		Effects::BasicFX->SetDiffuseMap(mGridDiffuseMapSRV);

		md3dImmediateContext->OMSetBlendState(RenderStates::TransparentBlendFactorBS, blendFactor, 0xffffffff);
		activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

		md3dImmediateContext->DrawIndexed(mGridIndexCount, mGridIndexOffset, mGridVertexOffset);

		// Restore default render state.
		md3dImmediateContext->RSSetState(0);

		// Restore default blend state
		md3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);
		md3dImmediateContext->OMSetDepthStencilState(0, 0);
	}

	// draw grass
	//activeTech = Effects::BasicFX->Light2TexTech;
	//activeTech->GetDesc(&techDesc);
	//for (UINT p = 0; p < techDesc.Passes; ++p)
	//{
	//	ID3DX11EffectPass* pass = activeTech->GetPassByIndex(p);

	//	md3dImmediateContext->IASetVertexBuffers(0, 1, &mGrassGridQuadVB, &stride, &offset);
	//	md3dImmediateContext->IASetIndexBuffer(mGrassGridQuadIB, DXGI_FORMAT_R32_UINT, 0);

	//	// Set per object constants.
	//	XMMATRIX world = XMLoadFloat4x4(&mGrassGridWorld);
	//	XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
	//	XMMATRIX worldViewProj = world * view * proj;

	//	Effects::BasicFX->SetWorld(world);
	//	Effects::BasicFX->SetWorldInvTranspose(worldInvTranspose);
	//	Effects::BasicFX->SetWorldViewProj(worldViewProj);
	//	Effects::BasicFX->SetTexTransform(XMMatrixIdentity());
	//	Effects::BasicFX->SetMaterial(mBoxMat);
	//	Effects::BasicFX->SetDiffuseMap(mGrassGridDiffuseMapSRV);		
	//	activeTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

	//	md3dImmediateContext->DrawIndexed(mGrassGridIndexCount, mGrassGridIndexOffset, mGrassGridVertexOffset);

	//	// Restore default render state.
	//	md3dImmediateContext->RSSetState(0);

	//	// Restore default blend state
	//	md3dImmediateContext->OMSetBlendState(0, blendFactor, 0xffffffff);
	//	md3dImmediateContext->OMSetDepthStencilState(0, 0);
	//}
#endif 

	HR(mSwapChain->Present(0, 0));
}

void CK_DX_StencilTextureBox::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void CK_DX_StencilTextureBox::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void CK_DX_StencilTextureBox::OnMouseMove(WPARAM btnState, int x, int y)
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

void CK_DX_StencilTextureBox::BuildGeometryBuffers()
{
	GeometryGenerator::MeshData box;

	GeometryGenerator geoGen;
	geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);

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

	//=================================================================
	// set up grid 
	// ================================================================
	// Create vertex buffer
	GeometryGenerator::MeshData gridMesh;

	GeometryGenerator geoGridGen;
	geoGridGen.CreateGrid(2.0f, 2.0f, 2, 2, gridMesh);

	mGridVertexOffset = 0;

	mGridIndexOffset = 0;

	mGridIndexCount = gridMesh.Indices.size();


	UINT totalGridVertexCount = gridMesh.Vertices.size();

	UINT totalGridIndexCount = mGridIndexCount;

	std::vector<Vertex::Basic32> gridVertices(totalGridVertexCount);

	k = 0;
	for (size_t i = 0; i < gridMesh.Vertices.size(); ++i, ++k)
	{
		gridVertices[k].Pos = gridMesh.Vertices[i].Position;
		gridVertices[k].Normal = gridMesh.Vertices[i].Normal;
		gridVertices[k].Tex = gridMesh.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC grid_vbd;
	grid_vbd.Usage = D3D11_USAGE_IMMUTABLE;
	grid_vbd.ByteWidth = sizeof(Vertex::Basic32) * totalGridVertexCount;
	grid_vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	grid_vbd.CPUAccessFlags = 0;
	grid_vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA grid_vinitData;
	grid_vinitData.pSysMem = &gridVertices[0];
	HR(md3dDevice->CreateBuffer(&grid_vbd, &grid_vinitData, &mGridQuadVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	std::vector<UINT> grid_indices;
	grid_indices.insert(grid_indices.end(), gridMesh.Indices.begin(), gridMesh.Indices.end());

	D3D11_BUFFER_DESC grid_ibd;
	grid_ibd.Usage = D3D11_USAGE_IMMUTABLE;
	grid_ibd.ByteWidth = sizeof(UINT) * totalGridIndexCount;
	grid_ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	grid_ibd.CPUAccessFlags = 0;
	grid_ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA grid_iinitData;
	grid_iinitData.pSysMem = &grid_indices[0];
	HR(md3dDevice->CreateBuffer(&grid_ibd, &grid_iinitData, &mGridQuadIB));


	//=================================================================
	// set up glass grid 
	// ================================================================
	// Create vertex buffer
	GeometryGenerator::MeshData grassGridMesh;

	GeometryGenerator geoGrassdGridGen;
	geoGrassdGridGen.CreateGrid(4.0f, 4.0f, 4, 4, grassGridMesh);

	mGrassGridVertexOffset = 0;
	mGrassGridIndexOffset = 0;
	mGrassGridIndexCount = grassGridMesh.Indices.size();

	UINT totalGrassGridVertexCount = grassGridMesh.Vertices.size();
	UINT totalGrassGridIndexCount = mGrassGridIndexCount;

	std::vector<Vertex::Basic32> grassGridVertices(totalGrassGridVertexCount);

	k = 0;
	for (size_t i = 0; i < grassGridMesh.Vertices.size(); ++i, ++k)
	{
		grassGridVertices[k].Pos = grassGridMesh.Vertices[i].Position;
		grassGridVertices[k].Normal = grassGridMesh.Vertices[i].Normal;
		grassGridVertices[k].Tex = grassGridMesh.Vertices[i].TexC;
	}

	D3D11_BUFFER_DESC grass_grid_vbd;
	grass_grid_vbd.Usage = D3D11_USAGE_IMMUTABLE;
	grass_grid_vbd.ByteWidth = sizeof(Vertex::Basic32) * totalGrassGridVertexCount;
	grass_grid_vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	grass_grid_vbd.CPUAccessFlags = 0;
	grass_grid_vbd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA grass_grid_vinitData;
	grass_grid_vinitData.pSysMem = &grassGridVertices[0];
	HR(md3dDevice->CreateBuffer(&grass_grid_vbd, &grass_grid_vinitData, &mGrassGridQuadVB));

	//
	// Pack the indices of all the meshes into one index buffer.
	//

	std::vector<UINT> grass_grid_indices;
	grass_grid_indices.insert(grass_grid_indices.end(), grassGridMesh.Indices.begin(), grassGridMesh.Indices.end());

	D3D11_BUFFER_DESC grass_grid_ibd;
	grass_grid_ibd.Usage = D3D11_USAGE_IMMUTABLE;
	grass_grid_ibd.ByteWidth = sizeof(UINT) * totalGrassGridIndexCount;
	grass_grid_ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	grass_grid_ibd.CPUAccessFlags = 0;
	grass_grid_ibd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA grass_grid_iinitData;
	grass_grid_iinitData.pSysMem = &grass_grid_indices[0];
	HR(md3dDevice->CreateBuffer(&grass_grid_ibd, &grass_grid_iinitData, &mGrassGridQuadIB));
}

