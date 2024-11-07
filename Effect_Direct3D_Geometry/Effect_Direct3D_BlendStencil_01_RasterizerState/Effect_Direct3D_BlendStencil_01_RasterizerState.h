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


class CK_DX_RasterizerStateBox : public D3DApp
{
public:
	CK_DX_RasterizerStateBox(HINSTANCE hInstance);
	~CK_DX_RasterizerStateBox();

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
	ID3D11Buffer* mBoxVB;
	ID3D11Buffer* mBoxIB;

	ID3D11ShaderResourceView* mDiffuseMapSRV;

	DirectionalLight mDirLights[3];
	Material mBoxMat;

	XMFLOAT4X4 mTexTransform;
	XMFLOAT4X4 mBoxWorld;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	int mBoxVertexOffset;
	UINT mBoxIndexOffset;
	UINT mBoxIndexCount;

	XMFLOAT3 mEyePosW;

	float mTheta;
	float mPhi;
	float mDeltaTime;
	float mRadius;

	POINT mLastMousePos;
};
