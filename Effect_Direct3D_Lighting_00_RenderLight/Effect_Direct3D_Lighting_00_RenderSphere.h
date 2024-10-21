#pragma once

#include "resource.h"
#include "d3dApp.h"
//#include "DDSTextureLoader.h"
#include "d3dx11Effect.h"
#include "GeometryGenerator.h"
#include "MathHelper.h"
#include "LightHelper.h"
#include "Effects.h"
#include "Vertex.h"


class CKDX_LightingGeo : public D3DApp
{
public:
	CKDX_LightingGeo(HINSTANCE hInstance);
	~CKDX_LightingGeo();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildGeometryBuffers();
	// 2024.10.06 start
	void BuildFX();
	void BuildVertexLayout();

private:
	ID3D11Buffer* mGeoVB;
	ID3D11Buffer* mGeoIB;

	ID3D11ShaderResourceView* mDiffuseMapSRV;

	//DirectionalLight mDirLights[3];
	DirectionalLight mDirLights;
	PointLight mPointLight;
	SpotLight mSpotLight;
	Material mGeoMat;

	// 2024.10.06 start	
	ID3DX11EffectTechnique* mfxTech;
	ID3DX11EffectMatrixVariable* mfxWorldViewProj;
	ID3DX11EffectMatrixVariable* mfxWorld;
	ID3DX11EffectMatrixVariable* mfxWorldInvTranspose;
	ID3DX11EffectVectorVariable* mfxEyePosW;
	ID3DX11EffectVariable* mfxDirLight;
	ID3DX11EffectVariable* mfxPointLight;
	ID3DX11EffectVariable* mfxSpotLight;
	ID3DX11EffectVariable* mfxMaterial;

	ID3D11InputLayout* mInputLayout;

	//XMFLOAT4X4 mTexTransform;
	XMFLOAT4X4 mGeoWorld;

	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	int mGeoVertexOffset;
	UINT mGeoIndexOffset;
	
	UINT mGeoIndexCount;

	XMFLOAT3 mEyePosW;

	float mTheta;
	float mPhi;
	float mRadius;

	POINT mLastMousePos;
};
