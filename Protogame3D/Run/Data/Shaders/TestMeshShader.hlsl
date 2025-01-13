struct ms_input_t
{
    float3 localPosition;
    float4 color;
    float2 uv;
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

static float4 cubeVertices[] =
{
    float4(-0.5f, -0.5f, -0.5f, 1.0f),
    float4(-0.5f, -0.5f,  0.5f, 1.0f),
    float4(-0.5f,  0.5f, -0.5f, 1.0f),
    float4(-0.5f,  0.5f,  0.5f, 1.0f),
    float4( 0.5f, -0.5f, -0.5f, 1.0f),
    float4( 0.5f, -0.5f,  0.5f, 1.0f),
    float4( 0.5f,  0.5f, -0.5f, 1.0f),
    float4( 0.5f,  0.5f,  0.5f, 1.0f)
};

static float4 cubeColor[] =
{
    float4(0.0f, 0.0f, 0.0f, 1.0f),
    float4(0.0f, 0.0f, 1.0f, 1.0f),
    float4(0.0f, 1.0f, 0.0f, 1.0f),
    float4(0.0f, 1.0f, 1.0f, 1.0f),
    float4(1.0f, 0.0f, 0.0f, 1.0f),
    float4(1.0f, 0.0f, 1.0f, 1.0f),
    float4(1.0f, 1.0f, 0.0f, 1.0f),
    float4(1.0f, 1.0f, 1.0f, 1.0f)
};

static uint3 cubeIndices[] =
{
    uint3(0, 2, 1),
    uint3(1, 2, 3),
    uint3(4, 5, 6),
    uint3(5, 7, 6),
    uint3(0, 1, 5),
    uint3(0, 5, 4),
    uint3(2, 6, 7),
    uint3(2, 7, 3),
    uint3(0, 4, 6),
    uint3(0, 6, 2),
    uint3(1, 3, 7),
    uint3(1, 7, 5)
};

[outputtopology("triangle")]
[numthreads(12, 1, 1)]
void MeshMain( 
        in uint                 grpID           : SV_GroupID,
        in uint                 grpThreadID     : SV_GroupThreadID, 
        out vertices m2p_t      outVerts[8], 
        out indices uint3       outIndices[12] )
{
    SetMeshOutputCounts(8, 12);

    if(grpThreadID < 8)
    {
        float4 pos = cubeVertices[grpThreadID];
        
        outVerts[grpThreadID].position = TransformPosition(pos.x, pos.y, pos.z);
        outVerts[grpThreadID].color = cubeColor[grpThreadID];
        outVerts[grpThreadID].uv = float2(0, 0);
    }

    outIndices[grpThreadID] = cubeIndices[grpThreadID];
}

[earlydepthstencil]
float4 PixelMain(m2p_t input) : SV_Target0
{
	return float4(input.color) * ModelColor;
}