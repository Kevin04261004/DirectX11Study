//=============================================================================
// Basic.fx by Frank Luna (C) 2011 All Rights Reserved.
//
// Basic effect that currently supports transformations, lighting, and texturing.
//=============================================================================

#include "LightHelper.fx"
 
cbuffer cbPerFrame
{
    DirectionalLight gDirLights[3];
    float3 gEyePosW;
    float gTime;
    float gFogStart;
    float gFogRange;
    float4 gFogColor;
};

cbuffer cbPerObject
{
    float4x4 gWorld;
    float4x4 gWorldInvTranspose;
    float4x4 gWorldViewProj;
    float4x4 gTexTransform;
    Material gMaterial;
}; 

// Nonnumeric values cannot be added to a cbuffer.
Texture2D gDiffuseMap;

SamplerState samAnisotropic
{
    Filter = ANISOTROPIC;
    MaxAnisotropy = 4;

    AddressU = WRAP;
    AddressV = WRAP;
};

struct VertexIn
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 Tex : TEXCOORD;
};

// for geometry shader
struct VertexToGeo
{
    float3 PosL : POSITION;
    float3 NormalL : NORMAL;
    float2 Tex : TEXCOORD;
};

// for geometry shader
struct GeoOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
    float3 NormalW : NORMAL;
    float2 Tex : TEXCOORD;
};

// for geometry shader2
struct VertexNormalOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
};

struct GeoNormalOut
{
    float4 PosH : SV_POSITION;
    float3 PosW : POSITION;
};


// vertex shader & geometry shader
VertexToGeo VSGeo(VertexIn vin)
{
    VertexToGeo vout;
		
    vout.PosL = vin.PosL;
    vout.NormalL = vin.NormalL;
	
	// Output vertex attributes for interpolation across triangle.
    vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

    return vout;
}

void Subdivide(VertexToGeo inVerts[3], out VertexToGeo outVerts[6])
{
    VertexToGeo m[3];
	// Compute edge midpoints in local space.
    m[0].PosL = 0.5f * (inVerts[0].PosL + inVerts[1].PosL);
    m[1].PosL = 0.5f * (inVerts[1].PosL + inVerts[2].PosL);
    m[2].PosL = 0.5f * (inVerts[2].PosL + inVerts[0].PosL);

	// Compute normals.
    m[0].NormalL = normalize(0.5f * (inVerts[0].NormalL + inVerts[1].NormalL));
    m[1].NormalL = normalize(0.5f * (inVerts[1].NormalL + inVerts[2].NormalL));
    m[2].NormalL = normalize(0.5f * (inVerts[2].NormalL + inVerts[0].NormalL));

	// Interpolate texture coordinates.
    m[0].Tex = 0.5f * (inVerts[0].Tex + inVerts[1].Tex);
    m[1].Tex = 0.5f * (inVerts[1].Tex + inVerts[2].Tex);
    m[2].Tex = 0.5f * (inVerts[2].Tex + inVerts[0].Tex);
    outVerts[0] = inVerts[0];
    outVerts[1] = m[0];
    outVerts[2] = m[2];
    outVerts[3] = m[1];
    outVerts[4] = inVerts[2];
    outVerts[5] = inVerts[1];
};

void OutputSubdivision(VertexToGeo v[6], inout TriangleStream<GeoOut> triStream)
{
    GeoOut gout[6];
	[unroll]
    for (int i = 0; i < 6; ++i)
    {
		// Transform to world space space.
        gout[i].PosW = mul(float4(v[i].PosL, 1.0f), gWorld).xyz;
        gout[i].NormalW = mul(v[i].NormalL, (float3x3) gWorldInvTranspose);

		// Transform to homogeneous clip space.
        gout[i].PosH = mul(float4(v[i].PosL, 1.0f), gWorldViewProj);
        gout[i].Tex = v[i].Tex;
    }
	// We can draw the subdivision in two strips: 
	// Strip 1: bottom three triangles
	// Strip 2: top triangle 
	[unroll]
    for (int j = 0; j < 5; ++j)
    {
        triStream.Append(gout[j]); // 0, 1, 2, 3, 4
        // 0 1 2
        // 1 3 2
        // 2 3 4
    }
    triStream.RestartStrip();
    triStream.Append(gout[1]); // 1
    triStream.Append(gout[5]); // 5
    triStream.Append(gout[3]); // 3
    // 1 5 3
};

[maxvertexcount(8)]
void GS(triangle VertexToGeo gin[3], inout TriangleStream<GeoOut> triStream)
{
    VertexToGeo v[6];
    Subdivide(gin, v);
    OutputSubdivision(v, triStream);
}

// normal pxiel shader & geometry shader
[maxvertexcount(8)]
void GSNormal(triangle VertexToGeo gin[3], inout LineStream<GeoNormalOut> lineStream)
{
    GeoNormalOut v1, v2;
    float normallength = 0.3;
    float3 tmpNormal;

    for (int i = 0; i < 3; i++)
    {
        v1.PosW = mul(float4(gin[i].PosL, 1.0f), gWorld).xyz;
        v1.PosH = mul(float4(gin[i].PosL, 1.0f), gWorldViewProj);
        lineStream.Append(v1);

        tmpNormal = gin[i].PosL + gin[i].NormalL * normallength;
        v2.PosW = mul(float4(tmpNormal, 1.0f), gWorld).xyz;
        v2.PosH = mul(float4(tmpNormal, 1.0f), gWorldViewProj);
        lineStream.Append(v2);

        lineStream.RestartStrip();
    }
}

[maxvertexcount(8)]
void GSNormal2(triangle VertexToGeo gin[3], inout LineStream<GeoNormalOut> lineStream)
{
    GeoNormalOut v1, v2;
    float normallength = 0.3;

    // 면의 중심을 계산합니다.
    float3 baseline1 = (gin[2].PosL - gin[1].PosL);
    float3 baseline2 = (gin[0].PosL - gin[1].PosL);
    float3 faceNormal = normalize(cross(baseline1, baseline2));
    float3 facePosL = (gin[0].PosL + gin[1].PosL + gin[2].PosL) / 3.0f; // 면의 중심 위치

    // 면의 중심에서 노말을 시작합니다.
    v1.PosW = mul(float4(facePosL, 1.0f), gWorld).xyz;
    v1.PosH = mul(float4(facePosL, 1.0f), gWorldViewProj);
    lineStream.Append(v1);

    // 노말 방향으로 일정 길이만큼 이동한 위치를 설정합니다.
    v2.PosW = mul(float4(facePosL + faceNormal * normallength, 1.0f), gWorld).xyz;
    v2.PosH = mul(float4(facePosL + faceNormal * normallength, 1.0f), gWorldViewProj);
    lineStream.Append(v2);

    lineStream.RestartStrip();
}

[maxvertexcount(8)]
void GSExplode(triangle VertexToGeo gin[3], inout TriangleStream<GeoOut> triStream)
{
    float deltaVal = 1.0;
    float3 baseline1 = (gin[2].PosL - gin[1].PosL);
    float3 baseline2 = (gin[0].PosL - gin[1].PosL);
    float3 faceNormal = normalize(cross(baseline1, baseline2));
    float3 facePosL;
    GeoOut gout[3];
    [unroll]
    for (int i = 0; i < 3; ++i) {
        facePosL = gin[i].PosL + faceNormal * ((sin(gTime) + 1.0) / 2.0) * deltaVal * gTime;
        gout[i].PosW= mul(float4(facePosL, 1.0f), gWorld).xyz;
        gout[i].NormalW= mul(gin[i].NormalL, (float3x3)gWorldInvTranspose);
        gout[i].PosH= mul(float4(facePosL, 1.0f), gWorldViewProj);
        gout[i].Tex = gin[i].Tex;
    }
    triStream.Append(gout[0]);
    triStream.Append(gout[1]);
    triStream.Append(gout[2]);
    triStream.RestartStrip();
}

[maxvertexcount(24)]
void GSOctahedron(triangle VertexToGeo gin[3], inout TriangleStream<GeoOut> triStream)
{
    float center;
    float edgeLength;
    
    float length01 = length(gin[1].PosL - gin[0].PosL);
    float length12 = length(gin[1].PosL - gin[2].PosL);
    float length20 = length(gin[2].PosL - gin[0].PosL);
    
    if (length01 < length12)
    {
        center = (gin[1].PosL + gin[2].PosL) / 2.0f;
        edgeLength = length12;
    }
    else if (length12 < length20)
    {
        center = (gin[2].PosL + gin[0].PosL) / 2.0f;
        edgeLength = length20;
    }
    else
    {
        center = (gin[0].PosL + gin[1].PosL) / 2.0f;
        edgeLength = length01;
    }
    
    // 정삼각형의 높이 계산
    float3 heightValue = float3(0, edgeLength * 0.5f, 0);
    float3 top = center + heightValue;
    if (top.x == 0)
    {
        //top = center2 + height2;
    }
    float3 bottom = center - heightValue; // 아래쪽으로 높이 1 추가
    if (top.x == 0)
    {
        //top = center2 - height2;
    }
    
    // GeoOut 구조체 배열 생성
    GeoOut gout[5];

    // 기존 정점 및 중심의 Homogeneous Clip Space 변환
    for (int i = 0; i < 3; ++i)
    {
        gout[i].PosH = mul(float4(gin[i].PosL, 1.0f), gWorldViewProj);
    }
    gout[3].PosH = mul(float4(top, 1.0f), gWorldViewProj); // 위쪽
    gout[4].PosH = mul(float4(bottom, 1.0f), gWorldViewProj); // 아래쪽

    // 위쪽 삼각형 생성
    for (int i = 0; i < 3; ++i)
    {
        triStream.Append(gout[i]); // 삼각형 정점 1
        triStream.Append(gout[3]); // 위쪽 꼭짓점
        triStream.Append(gout[(i + 1) % 3]); // 삼각형 정점 2
        triStream.RestartStrip();
    }

    // 아래쪽 삼각형 생성
    for (int i = 0; i < 3; ++i)
    {
        triStream.Append(gout[i]); // 삼각형 정점 1
        triStream.Append(gout[4]); // 아래쪽 꼭짓점
        triStream.Append(gout[(i + 1) % 3]); // 삼각형 정점 2
        triStream.RestartStrip();
    }
}

float4 PSNormal(GeoNormalOut pin) : SV_Target
{
	// Default to multiplicative identity.	
    return float4(1, 0, 0, 1);;
}

float4 PSNormal2(GeoNormalOut pin) : SV_Target
{
	// Default to multiplicative identity.	
    return float4(0, 1, 0, 1);;
}

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
	
	// Transform to world space space.
    vout.PosW = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
    vout.NormalW = mul(vin.NormalL, (float3x3) gWorldInvTranspose);
		
	// Transform to homogeneous clip space.
    vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	
	// Output vertex attributes for interpolation across triangle.
    vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

    return vout;
}
 
float4 PS(VertexOut pin, uniform int gLightCount, uniform bool gUseTexure, uniform bool gAlphaClip) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
    pin.NormalW = normalize(pin.NormalW);

	// The toEye vector is used in lighting.
    float3 toEye = gEyePosW - pin.PosW;

	// Cache the distance to the eye from this surface point.
    float distToEye = length(toEye);

	// Normalize.
    toEye /= distToEye;
	
    // Default to multiplicative identity.
    float4 texColor = float4(1, 1, 1, 1);
    if (gUseTexure)
    {
		// Sample texture.
        texColor = gDiffuseMap.Sample(samAnisotropic, pin.Tex);
        if (gAlphaClip)
        {
			// Discard pixel if texture alpha < 0.1.  Note that we do this
			// test as soon as possible so that we can potentially exit the shader 
			// early, thereby skipping the rest of the shader code.
            clip(texColor.a - 0.1f);
        }
    }
	 
	//
	// Lighting.
	//

    float4 litColor = texColor;
    if (gLightCount > 0)
    {
		// Start with a sum of zero. 
        float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
        float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
        float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

		// Sum the light contribution from each light source.  
		[unroll]
        for (int i = 0; i < gLightCount; ++i)
        {
            float4 A, D, S;
            ComputeDirectionalLight(gMaterial, gDirLights[i], pin.NormalW, toEye,
				A, D, S);

            ambient += A;
            diffuse += D;
            spec += S;
        }

		// Modulate with late add.
        litColor = texColor * (ambient + diffuse) + spec;
    }

	// Common to take alpha from diffuse material and texture.
    litColor.a = gMaterial.Diffuse.a * texColor.a;

    return litColor;
}

technique11 Light1
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, false, false)));
    }
}

technique11 Light2
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, false, false)));
    }
}

technique11 Light3
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, false, false)));
    }
}

technique11 Light0Tex
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(0, true, false)));
    }
}

technique11 Light1Tex
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, true, false)));
    }
}

technique11 Light2Tex
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, true, false)));
    }
}

technique11 Light3Tex
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, true, false)));
    }
}

technique11 Light0TexAlphaClip
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(0, true, true)));
    }
}

technique11 Light1TexAlphaClip
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(1, true, true)));
    }
}

technique11 Light2TexAlphaClip
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(2, true, true)));
    }
}

technique11 Light3TexAlphaClip
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_5_0, PS(3, true, true)));
    }
}


technique11 Light0TexGeo
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VSGeo()));
        SetGeometryShader(CompileShader(gs_5_0, GS()));
        SetPixelShader(CompileShader(ps_5_0, PS(0, true, false)));
    }
}

technique11 Light1TexGeo
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VSGeo()));
        SetGeometryShader(CompileShader(gs_5_0, GS()));
        SetPixelShader(CompileShader(ps_5_0, PS(1, true, false)));
    }
}


technique11 Light2TexGeo
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VSGeo()));
        SetGeometryShader(CompileShader(gs_5_0, GS()));
        SetPixelShader(CompileShader(ps_5_0, PS(2, true, false)));
    }
}

technique11 GeoOctahedron
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VSGeo()));
        SetGeometryShader(CompileShader(gs_5_0, GSOctahedron()));
        SetPixelShader(CompileShader(ps_5_0, PSNormal()));
    }
}

technique11 GeoNormal
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VSGeo()));
        SetGeometryShader(CompileShader(gs_5_0, GSNormal()));
        SetPixelShader(CompileShader(ps_5_0, PSNormal()));
    }
}


technique11 GeoNormal2
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VSGeo()));
        SetGeometryShader(CompileShader(gs_5_0, GSNormal2()));
        SetPixelShader(CompileShader(ps_5_0, PSNormal2()));
    }
}

technique11 GeoExplode
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_5_0, VSGeo()));
        SetGeometryShader(CompileShader(gs_5_0, GSExplode()));
        SetPixelShader(CompileShader(ps_5_0, PS(0, true, false)));
    }
}