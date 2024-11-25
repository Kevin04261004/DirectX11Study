struct Data
{
	int colorVal;
};

StructuredBuffer<Data> gInputA;
RWStructuredBuffer<Data> gOutput;

float3 readPixel(int x, int y)
{
	float3 output;
	//uint index = (x + y * 1024);	
	uint index = (x + y * 512);

	output.x = (float)(((gInputA[index].colorVal) & 0x000000ff)) / 255.0f;
	output.y = (float)(((gInputA[index].colorVal) & 0x0000ff00) >> 8) / 255.0f;
	output.z = (float)(((gInputA[index].colorVal) & 0x00ff0000) >> 16) / 255.0f;

	return output;
}

void writeToPixel(int x, int y, float3 colorVal)
{
	uint index = (x + y * 512);

	int ired = (int)(clamp(colorVal.r, 0, 1) * 255);
	int igreen = (int)(clamp(colorVal.g, 0, 1) * 255) << 8;
	int iblue = (int)(clamp(colorVal.b, 0, 1) * 255) << 16;

	gOutput[index].colorVal = ired + igreen + iblue;
}


// [numthreads(32, 16, 1)]
[numthreads(32, 32, 1)]
void CS(int3 dtid : SV_DispatchThreadID)
{
	float3 pixel = readPixel(dtid.x, dtid.y);
	pixel.rgb = pixel.r * 0.3 + pixel.g * 0.59 + pixel.b * 0.11;
	writeToPixel(dtid.x, dtid.y, pixel);
}

technique11 TextureTech 
{
    pass P0
    {
		SetVertexShader( NULL );
        SetPixelShader( NULL );
		SetComputeShader( CompileShader( cs_5_0, CS() ) );
    }
}
