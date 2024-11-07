//***************************************************************************************
// Effects.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "Effects.h"

#pragma region Effect
Effect::Effect(ID3D11Device* device, const std::wstring& filename)
	: mFX(0)
{
	// �����Ͽ� wstring�� ���� LPCWSTR�� ���� 
	LPCWSTR lpcwstr = filename.c_str();

	DWORD shaderFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* compiledErrorMsg = 0;

	// D3D_COMPILE_STANDARD_FILE_INCLUDE �� ���ؼ� fx���� �ȿ� include ���� ó�� 
	HRESULT hr = D3DX11CompileEffectFromFile(lpcwstr,
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		shaderFlags,
		0,
		device,
		&mFX,
		&compiledErrorMsg);

	// compiledErrorMsg can store errors or warnings.
	if (compiledErrorMsg != 0)
	{
		MessageBoxA(0, (char*)compiledErrorMsg->GetBufferPointer(), 0, 0);
		ReleaseCOM(compiledErrorMsg);
	}

	// Even if there are no compiledErrorMsg, check to make sure there were no other errors.
	if (FAILED(hr))
	{
		MessageBoxA(0, "Error D3DX11CompileEffectFromFile", 0, 0);
	}
}

Effect::~Effect()
{
	ReleaseCOM(mFX);
}
#pragma endregion

#pragma region BasicEffect
BasicEffect::BasicEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	Light1Tech    = mFX->GetTechniqueByName("Light1");
	Light2Tech    = mFX->GetTechniqueByName("Light2");
	Light3Tech    = mFX->GetTechniqueByName("Light3");

	Light0TexTech = mFX->GetTechniqueByName("Light0Tex");
	Light1TexTech = mFX->GetTechniqueByName("Light1Tex");
	Light2TexTech = mFX->GetTechniqueByName("Light2Tex");
	Light3TexTech = mFX->GetTechniqueByName("Light3Tex");

	Light0TexAlphaClipTech = mFX->GetTechniqueByName("Light0TexAlphaClip");
	Light1TexAlphaClipTech = mFX->GetTechniqueByName("Light1TexAlphaClip");
	Light2TexAlphaClipTech = mFX->GetTechniqueByName("Light2TexAlphaClip");
	Light3TexAlphaClipTech = mFX->GetTechniqueByName("Light3TexAlphaClip");

	Light0TexGeoTech = mFX->GetTechniqueByName("Light0TexGeo");
	Light1TexGeoTech = mFX->GetTechniqueByName("Light1TexGeo");
	Light2TexGeoTech = mFX->GetTechniqueByName("Light2TexGeo");

	TexGeoNormalTech = mFX->GetTechniqueByName("GeoNormal");
	TexGeoNormal2Tech = mFX->GetTechniqueByName("GeoNormal2");

	TexGeoExplodeTech = mFX->GetTechniqueByName("GeoExplode");

	WorldViewProj     = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	World             = mFX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	TexTransform      = mFX->GetVariableByName("gTexTransform")->AsMatrix();
	EyePosW           = mFX->GetVariableByName("gEyePosW")->AsVector();
	DirLights         = mFX->GetVariableByName("gDirLights");
	Mat               = mFX->GetVariableByName("gMaterial");
	DiffuseMap        = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	TimeVal			  = mFX->GetVariableByName("gTime")->AsScalar();
}

BasicEffect::~BasicEffect()
{
}
#pragma endregion

#pragma region Effects

BasicEffect* Effects::BasicFX = 0;

void Effects::InitAll(ID3D11Device* device)
{
	//BasicFX = new BasicEffect(device, L"FX/Basic.fx");
	BasicFX = new BasicEffect(device, L"FX/BasicGeoNormalExt.fx");
	//BasicFX = new BasicEffect(device, L"FX/Basic.fxo");
}

void Effects::DestroyAll()
{
	SafeDelete(BasicFX);
}
#pragma endregion