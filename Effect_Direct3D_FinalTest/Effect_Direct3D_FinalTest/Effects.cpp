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
	Light0TexTech = mFX->GetTechniqueByName("Light0Tex");
	VisibleNormalTech = mFX->GetTechniqueByName("GeoNormal");
	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	World = mFX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	TexTransform = mFX->GetVariableByName("gTexTransform")->AsMatrix();
	EyePosW = mFX->GetVariableByName("gEyePosW")->AsVector();
	FogColor = mFX->GetVariableByName("gFogColor")->AsVector();
	FogStart = mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange = mFX->GetVariableByName("gFogRange")->AsScalar();
	DirLights = mFX->GetVariableByName("gDirLights");
	Mat = mFX->GetVariableByName("gMaterial");
	DiffuseMap = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
}

BasicEffect::~BasicEffect()
{
}
#pragma endregion

#pragma region TexturedEffect
TexturedEffect::TexturedEffect(ID3D11Device* device, const std::wstring& filename)
	: Effect(device, filename)
{
	TextureEffectTech = mFX->GetTechniqueByName("TextureTech");

	InputA = mFX->GetVariableByName("gInputA")->AsShaderResource();
	Output = mFX->GetVariableByName("gOutput")->AsUnorderedAccessView();
}

TexturedEffect::~TexturedEffect()
{
}
#pragma endregion

#pragma region Effects

BasicEffect* Effects::BasicFX = 0;
TexturedEffect* Effects::TexturedFX = 0;

void Effects::InitAll(ID3D11Device* device)
{	
	BasicFX = new BasicEffect(device, L"FX/Basic.fx");			
	TexturedFX = new TexturedEffect(device, L"FX/Textured.fx");
}

void Effects::DestroyAll()
{
	SafeDelete(BasicFX);		
	SafeDelete(TexturedFX);
}
#pragma endregion