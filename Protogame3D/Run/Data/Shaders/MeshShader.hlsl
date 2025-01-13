//------------------------------------------------------------------------------------------------
Texture2D srcTexture : register(t0);
SamplerState diffuseSampler : register(s0);

struct ms_input_t
{
    float3 localPosition : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct m2p_t
{
	float4 position : SV_Position;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};

cbuffer ModelConstants : register(b1)
{	
    float4 ModelColor;
    float4x4 ModelMatrix;
};

cbuffer CameraConstants : register(b0)
{
    float4x4 ViewMatrix;
    float4x4 ProjectionMatrix;
};

float4 TransformPosition(float x, float y, float z)
{
    float4 localPosition = float4(x, y, z, 1);

    float4 worldPosition = mul(ModelMatrix, localPosition);
    float4 viewPosition = mul(ViewMatrix, worldPosition);
    float4 clipPosition = mul(ProjectionMatrix, viewPosition);
	
    return clipPosition;
}

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void MeshMain( out vertices ms_input_t outVerts[8], out indices uint3 outIndices[12] )
{
    const uint numVertices = 8;
    const uint numPrimitives = 12;
    
    SetMeshOutputCounts(numVertices, numPrimitives);
    
    outVerts[0].localPosition = TransformPosition(-0.5f, -0.5f, -0.5f);
    outVerts[0].color = float4(0.0f, 0.0f, 0.0f, 1.0f);
    outVerts[0].uv = float2(0.0f, 0.0f);
    
    outVerts[1].localPosition = TransformPosition(-0.5f, -0.5f, 0.5f);
    outVerts[1].color = float4(0.0f, 0.0f, 1.0f, 1.0f);
    outVerts[1].uv = float2(0.0f, 0.0f);
    
    outVerts[2].localPosition = TransformPosition(-0.5f, 0.5f, -0.5f);
    outVerts[2].color = float4(0.0f, 1.0f, 0.0f, 1.0f);
    outVerts[2].uv = float2(0.0f, 0.0f);
    
    outVerts[3].localPosition = TransformPosition(-0.5f, 0.5f, 0.5f);
    outVerts[3].color = float4(0.0f, 1.0f, 1.0f, 1.0f);
    outVerts[3].uv = float2(0.0f, 0.0f);
    
    outVerts[4].localPosition = TransformPosition(0.5f, -0.5f, -0.5f);
    outVerts[4].color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    outVerts[4].uv = float2(0.0f, 0.0f);
    
    outVerts[5].localPosition = TransformPosition(0.5f, -0.5f, 0.5f);
    outVerts[5].color = float4(1.0f, 0.0f, 1.0f, 1.0f);
    outVerts[5].uv = float2(0.0f, 0.0f);
    
    outVerts[6].localPosition = TransformPosition(0.5f, 0.5f, -0.5f);
    outVerts[6].color = float4(1.0f, 1.0f, 0.0f, 1.0f);
    outVerts[6].uv = float2(0.0f, 0.0f);
    
    outVerts[7].localPosition = TransformPosition(0.5f, 0.5f, 0.5f);
    outVerts[7].color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    outVerts[7].uv = float2(0.0f, 0.0f);
    
    outIndices[0] = uint3(0, 2, 1);
    outIndices[1] = uint3(1, 2, 3);
    outIndices[2] = uint3(4, 5, 6);
    outIndices[3] = uint3(5, 7, 6);
    outIndices[4] = uint3(0, 1, 5);
    outIndices[5] = uint3(0, 5, 4);
    outIndices[6] = uint3(2, 6, 7);
    outIndices[7] = uint3(2, 7, 3);
    outIndices[8] = uint3(0, 4, 6);
    outIndices[9] = uint3(0, 6, 2);
    outIndices[10] = uint3(1, 3, 7);
    outIndices[11] = uint3(1, 7, 5);
}

float4 PixelMain(m2p_t input) : SV_Target0
{
	float4 textureColor = srcTexture.Sample(diffuseSampler, input.uv);
	return float4(input.color) * ModelColor * textureColor;
}