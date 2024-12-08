#pragma once

#include "resource.h"
#include "d3dApp.h"
#include "d3dx11Effect.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "Effects.h"
#include "Vertex.h"
#include "Camera.h"
#include "RenderStates.h"
#include "TextureMgr.h"
#include "BasicModel.h"
#include "SkinnedModel.h"


struct BoundingSphere
{
	BoundingSphere() : Center(0.0f, 0.0f, 0.0f), Radius(0.0f) {}
	XMFLOAT3 Center;
	float Radius;
};

class CK_DX_BasicSkinnedMesh : public D3DApp
{
public:
	CK_DX_BasicSkinnedMesh(HINSTANCE hInstance);
	~CK_DX_BasicSkinnedMesh();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	TextureMgr mTexMgr;

	SkinnedModel* mCharacterModel;
	SkinnedModelInstance mCharacterInstance[10];

	XMFLOAT4X4 mLightView;
	XMFLOAT4X4 mLightProj;

	DirectionalLight mDirLights[3];

	Camera mCam;
	POINT mLastMousePos;
};
