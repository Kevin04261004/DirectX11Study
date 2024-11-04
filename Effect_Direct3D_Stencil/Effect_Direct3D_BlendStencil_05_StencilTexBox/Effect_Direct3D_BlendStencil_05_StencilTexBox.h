#pragma once

#include "resource.h"
#include "d3dApp.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"
#include "d3dx11Effect.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "RenderStates.h"
#include "Effects.h"
#include "Vertex.h"


class CK_DX_StencilTextureBox : public D3DApp
{
public:
	CK_DX_StencilTextureBox(HINSTANCE hInstance);
	~CK_DX_StencilTextureBox();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffers();

private:
	ID3D11Buffer* mGrassGridQuadVB;
	ID3D11Buffer* mGrassGridQuadIB;

	ID3D11Buffer* mGridQuadVB;
	ID3D11Buffer* mGridQuadIB;

	ID3D11Buffer* mBoxVB;
	ID3D11Buffer* mBoxIB;

	ID3D11ShaderResourceView* mDiffuseMapSRV;
	ID3D11ShaderResourceView* mGridDiffuseMapSRV;
	ID3D11ShaderResourceView* mGrassGridDiffuseMapSRV;

	DirectionalLight mDirLights[3];
	Material mBoxMat;

	XMFLOAT4X4 mTexTransform;
	XMFLOAT4X4 mBoxWorld;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	XMFLOAT4X4 mGridWorld;
	int mGridVertexOffset;
	UINT mGridIndexOffset;
	UINT mGridIndexCount;

	XMFLOAT4X4 mGrassGridWorld;
	int mGrassGridVertexOffset;
	UINT mGrassGridIndexOffset;
	UINT mGrassGridIndexCount;

	int mBoxVertexOffset;
	UINT mBoxIndexOffset;
	UINT mBoxIndexCount;

	XMFLOAT3 mEyePosW;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};
