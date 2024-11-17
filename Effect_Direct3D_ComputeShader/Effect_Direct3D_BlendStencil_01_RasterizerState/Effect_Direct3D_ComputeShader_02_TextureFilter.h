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


class CK_DX_ComputeTexFilter : public D3DApp
{
public:
	CK_DX_ComputeTexFilter(HINSTANCE hInstance);
	~CK_DX_ComputeTexFilter();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

	void OnMouseDown(WPARAM btnState, int x, int y);
	void OnMouseUp(WPARAM btnState, int x, int y);
	void OnMouseMove(WPARAM btnState, int x, int y);

private:
	void BuildCrateGeometryBuffers();
	void BuildInBuffer();
	void BuildOutBuffer();

	void DoComputeWork();

private:
    ID3D11Buffer* mBoxVB;
    ID3D11Buffer* mBoxIB;

    byte* mSrcTextureData;
    ID3D11Texture2D* mSrcTexture;
    ID3D11ShaderResourceView* mTextureSRV;

    UINT mTextureDataSize;
    ID3D11ShaderResourceView* mInputASRV;

    ID3D11Buffer* mOutputBuffer;
    ID3D11UnorderedAccessView* mOutputUAV;

    ID3D11Texture2D* mDestTexture;
    ID3D11ShaderResourceView* mDestTextureView;

    XMFLOAT3 mEyePosW;
    float mTheta;
    float mPhi;
    float mRadius;

    XMFLOAT4X4 mView;
    XMFLOAT4X4 mProj;
    XMFLOAT4X4 mBoxWorld;

    Material mBoxMat;

    bool mUseCS;
    bool mKeyPressed;

    POINT mLastMousePos;
};
