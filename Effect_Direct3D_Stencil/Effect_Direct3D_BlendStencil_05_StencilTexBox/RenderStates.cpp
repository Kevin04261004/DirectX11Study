//***************************************************************************************
// RenderStates.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "RenderStates.h"

ID3D11RasterizerState* RenderStates::WireframeRS = 0;
ID3D11RasterizerState* RenderStates::NoCullWireframeRS = 0;
ID3D11RasterizerState* RenderStates::NoCullRS    = 0;
ID3D11RasterizerState* RenderStates::CullRS = 0;
ID3D11RasterizerState* RenderStates::CullFrontCCWRS = 0;
	 
ID3D11BlendState*      RenderStates::AlphaToCoverageBS = 0;
ID3D11BlendState*      RenderStates::TransparentBS     = 0;
ID3D11BlendState*	   RenderStates::TransparentBlendFactorBS = 0;
ID3D11BlendState*	   RenderStates::NoRenderTargetWritesBS = 0;

ID3D11DepthStencilState* RenderStates::NoDepthDS = 0;
ID3D11DepthStencilState* RenderStates::MarkDSS = 0;
ID3D11DepthStencilState* RenderStates::DrawDSS = 0;



void RenderStates::InitAll(ID3D11Device* device)
{
	//
	// WireframeRS
	//
	D3D11_RASTERIZER_DESC wireframeDesc;
	ZeroMemory(&wireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	wireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireframeDesc.CullMode = D3D11_CULL_BACK;
	wireframeDesc.FrontCounterClockwise = false;
	wireframeDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&wireframeDesc, &WireframeRS));


	//
	// NoCullWireframeRS
	//
	D3D11_RASTERIZER_DESC noCullWireframeDesc;
	ZeroMemory(&noCullWireframeDesc, sizeof(D3D11_RASTERIZER_DESC));
	noCullWireframeDesc.FillMode = D3D11_FILL_WIREFRAME;
	noCullWireframeDesc.CullMode = D3D11_CULL_NONE;
	noCullWireframeDesc.FrontCounterClockwise = false;
	noCullWireframeDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&noCullWireframeDesc, &NoCullWireframeRS));

	//
	// NoCullRS
	//
	D3D11_RASTERIZER_DESC noCullDesc;
	ZeroMemory(&noCullDesc, sizeof(D3D11_RASTERIZER_DESC));
	noCullDesc.FillMode = D3D11_FILL_SOLID;
	noCullDesc.CullMode = D3D11_CULL_NONE;
	noCullDesc.FrontCounterClockwise = false;
	noCullDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&noCullDesc, &NoCullRS));

	//
	// CullRS
	//
	D3D11_RASTERIZER_DESC cullDesc;
	ZeroMemory(&cullDesc, sizeof(D3D11_RASTERIZER_DESC));
	cullDesc.FillMode = D3D11_FILL_SOLID;
	cullDesc.CullMode = D3D11_CULL_BACK;
	cullDesc.FrontCounterClockwise = false;
	cullDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&cullDesc, &CullRS));

	D3D11_RASTERIZER_DESC cullFrontCCWDesc;
	ZeroMemory(&cullFrontCCWDesc, sizeof(D3D11_RASTERIZER_DESC));
	cullFrontCCWDesc.FillMode = D3D11_FILL_SOLID;
	cullFrontCCWDesc.CullMode = D3D11_CULL_BACK;
	cullFrontCCWDesc.FrontCounterClockwise = true;
	cullFrontCCWDesc.DepthClipEnable = true;

	HR(device->CreateRasterizerState(&cullFrontCCWDesc, &CullFrontCCWRS));


	//
	// AlphaToCoverageBS
	//

	D3D11_BLEND_DESC alphaToCoverageDesc = {0};
	alphaToCoverageDesc.AlphaToCoverageEnable = true;
	alphaToCoverageDesc.IndependentBlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].BlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(device->CreateBlendState(&alphaToCoverageDesc, &AlphaToCoverageBS));

	//
	// TransparentBS
	//

	D3D11_BLEND_DESC transparentDesc = {0};
	transparentDesc.AlphaToCoverageEnable = false;
	transparentDesc.IndependentBlendEnable = false;

	transparentDesc.RenderTarget[0].BlendEnable = true;
	transparentDesc.RenderTarget[0].SrcBlend       = D3D11_BLEND_SRC_ALPHA;
	transparentDesc.RenderTarget[0].DestBlend      = D3D11_BLEND_INV_SRC_ALPHA;
	transparentDesc.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	transparentDesc.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(device->CreateBlendState(&transparentDesc, &TransparentBS));

	//
	// TransparentBlendFactor BS
	//

	D3D11_BLEND_DESC transparentblendFactorDesc = { 0 };
	transparentblendFactorDesc.AlphaToCoverageEnable = false;
	transparentblendFactorDesc.IndependentBlendEnable = false;

	transparentblendFactorDesc.RenderTarget[0].BlendEnable = true;
	transparentblendFactorDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_COLOR;
	transparentblendFactorDesc.RenderTarget[0].DestBlend = D3D11_BLEND_BLEND_FACTOR;	

	transparentblendFactorDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	transparentblendFactorDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	transparentblendFactorDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	transparentblendFactorDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	transparentblendFactorDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	HR(device->CreateBlendState(&transparentblendFactorDesc, &TransparentBlendFactorBS));

	//
	// NoRenderTargetWritesBS
	//

	D3D11_BLEND_DESC noRenderTargetWritesDesc = { 0 };
	noRenderTargetWritesDesc.AlphaToCoverageEnable = false;
	noRenderTargetWritesDesc.IndependentBlendEnable = false;

	noRenderTargetWritesDesc.RenderTarget[0].BlendEnable = false;
	noRenderTargetWritesDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	noRenderTargetWritesDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	noRenderTargetWritesDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	noRenderTargetWritesDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	noRenderTargetWritesDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	noRenderTargetWritesDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	noRenderTargetWritesDesc.RenderTarget[0].RenderTargetWriteMask = 0;

	HR(device->CreateBlendState(&noRenderTargetWritesDesc, &NoRenderTargetWritesBS));


	// disable depth 	
	D3D11_DEPTH_STENCIL_DESC noDepthDesc;
	noDepthDesc.DepthEnable = true;
	noDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	noDepthDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	noDepthDesc.StencilEnable = false;
	noDepthDesc.StencilReadMask = 0xff;
	noDepthDesc.StencilWriteMask = 0xff;

	noDepthDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	noDepthDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDepthDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	noDepthDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	
	noDepthDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	noDepthDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	noDepthDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	noDepthDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	HR(device->CreateDepthStencilState(&noDepthDesc, &NoDepthDS));

	// 스텐실 버퍼에 렌더링 시 스텐실 테스트 항성 성공 하도록 설정 
	// 스텐실 테스트 성공 시 스텐실 버퍼 값이 1(stencil ref)로 설정
	// 스텐실 버퍼 갱신 방법을  D3D11_STENCIL_OP_REPLACE
	//	- 스텐실 버퍼의 값을 스텐실 기준 값으로 변경(StencilRef)
	//
	// 깊이 테스트 실패 시 스텐실 버퍼는 변하지 않도록 깊이 버퍼 갱신
	// 방법을 D3D11_STENCIL_OP_KEEP로 설정

	D3D11_DEPTH_STENCIL_DESC markDesc;
	markDesc.DepthEnable = true;
	markDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	markDesc.DepthFunc = D3D11_COMPARISON_LESS;
	markDesc.StencilEnable = true;
	markDesc.StencilReadMask = 0xff;
	markDesc.StencilWriteMask = 0xff;

	markDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	markDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	markDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	markDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// We are not rendering backfacing polygons, so these settings do not matter.
	markDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	markDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	markDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	markDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	HR(device->CreateDepthStencilState(&markDesc, &MarkDSS));

	//
	// Draw DSS
	//

	D3D11_DEPTH_STENCIL_DESC drawDesc;
	drawDesc.DepthEnable = true;
	drawDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	drawDesc.DepthFunc = D3D11_COMPARISON_LESS;
	drawDesc.StencilEnable = true;
	drawDesc.StencilReadMask = 0xff;
	drawDesc.StencilWriteMask = 0xff;

	drawDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	drawDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	drawDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	drawDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	// We are not rendering backfacing polygons, so these settings do not matter.
	drawDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	drawDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	drawDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	drawDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	HR(device->CreateDepthStencilState(&drawDesc, &DrawDSS));
}

void RenderStates::DestroyAll()
{
	ReleaseCOM(WireframeRS);
	ReleaseCOM(NoCullWireframeRS);
	ReleaseCOM(NoCullRS);
	ReleaseCOM(CullRS);
	ReleaseCOM(CullFrontCCWRS);

	
	ReleaseCOM(NoRenderTargetWritesBS);
	ReleaseCOM(AlphaToCoverageBS);
	ReleaseCOM(TransparentBS);
	ReleaseCOM(TransparentBlendFactorBS);	

	ReleaseCOM(NoDepthDS);
	ReleaseCOM(MarkDSS);
	ReleaseCOM(DrawDSS);	
}