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

cbuffer MeshletDebug : register(b2)
{
    int meshletVisualizer;
};

struct Meshlet
{
    uint vertices[64];
    uint3 primitives[42];
    float4 color;
};

StructuredBuffer<Meshlet> Meshlets : register(t0);
StructuredBuffer<ms_input_t> Vertices : register(t1);

float4 TransformPosition(float x, float y, float z)
{
    float4 localPosition = float4(x, y, z, 1);

    float4 worldPosition = mul(ModelMatrix, localPosition);
    float4 viewPosition = mul(ViewMatrix, worldPosition);
    float4 clipPosition = mul(ProjectionMatrix, viewPosition);
	
    return clipPosition;
}

[outputtopology("triangle")]
[numthreads(64, 1, 1)]
void MeshMain( 
        in uint                 grpID           : SV_GroupID,
        in uint                 grpThreadID     : SV_GroupThreadID, 
        out vertices m2p_t      outVerts[64], 
        out indices uint3       outIndices[42] )
{
    Meshlet meshlet = Meshlets[grpID];
    
    SetMeshOutputCounts(64, 42);
        
    if (grpThreadID < 64)
    {
        uint vi = meshlet.vertices[grpThreadID];
        
        ms_input_t vertex = Vertices[vi];
        
        outVerts[grpThreadID].position = TransformPosition(vertex.localPosition.x, vertex.localPosition.y, vertex.localPosition.z);
        outVerts[grpThreadID].color = meshletVisualizer == 0 ? vertex.color : meshlet.color;
        outVerts[grpThreadID].uv = vertex.uv;

    }
        
    if (grpThreadID < 42)
    {
        outIndices[grpThreadID] = meshlet.primitives[grpThreadID];
    }
}

[earlydepthstencil]
float4 PixelMain(m2p_t input) : SV_Target0
{
	return float4(input.color) * ModelColor;
}