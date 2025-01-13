#include "MeshletUtils.hlsli"

struct GrassPatchArguments
{
    float3 patchPosition;
    float3 groundNormal;
    float height;
};

int tsign(in uint gtid, in int id)
{
    return (gtid & (1u << id)) ? 1 : -1;
}

void MakePersistentLength(in float3 v0, inout float3 v1, inout float3 v2, in float height)
{
    //Persistent length
    float3 v01 = v1 - v0;
    float3 v12 = v2 - v1;
    float lv01 = length(v01);
    float lv12 = length(v12);

    float L1 = lv01 + lv12;
    float L0 = length(v2 - v0);
    float L = (2.0f * L0 + L1) / 3.0f;

    float ldiff = height / L;
    v01 = v01 * ldiff;
    v12 = v12 * ldiff;
    v1 = v0 + v01;
    v2 = v1 + v12;
}

float3 bezierDerivative(float3 p0, float3 p1, float3 p2, float t)
{
    return 2. * (1. - t) * (p1 - p0) + 2. * t * (p2 - p1);
}

float3 bezier(float3 p0, float3 p1, float3 p2, float t)
{
    float3 a = lerp(p0, p1, t);
    float3 b = lerp(p1, p2, t);
    return lerp(a, b, t);
}

static const int GROUP_SIZE = 128;
static const int GRASS_VERT_COUNT = 8;
static const int GRASS_PRIM_COUNT = 6;

static float3 GrassBlade[] =
{
    float3(-0.1f, 0.0f, 0.0f), //0
    float3( 0.1f, 0.0f, 0.0f), //1
    float3( 0.1f, 0.0f, 0.2f), //2
    float3(-0.1f, 0.0f, 0.2f), //3
    float3(-0.1f, 0.0f, 0.4f), //4
    float3( 0.1f, 0.0f, 0.4f), //5
    float3( 0.1f, 0.0f, 0.6f), //6
    float3(-0.1f, 0.0f, 0.6f)  //7
};

static uint3 GrassBladePrim[] =
{
    uint3(0, 1, 2), //0
    uint3(2, 3, 0), //1
    uint3(3, 2, 5), //2
    uint3(5, 4, 3), //3
    uint3(4, 5, 6), //4
    uint3(6, 7, 4), //5
};

[NumThreads(GROUP_SIZE, 1, 1)]
[OutputTopology("triangle")]
void MeshMain(
    uint gtid : SV_GroupThreadID,
    uint gid : SV_GroupID,
    out indices uint3 tris[GRASS_PRIM_COUNT],
    out vertices m2p_t verts[GRASS_VERT_COUNT]
)
{    
    SetMeshOutputCounts(GRASS_VERT_COUNT, GRASS_PRIM_COUNT);
	
    if (gtid < GRASS_VERT_COUNT)
    {
        float4 localPosition = float4(GrassBlade[gtid].x, GrassBlade[gtid].y, GrassBlade[gtid].z, 1);

        float4 worldPosition = mul(ModelMatrix, localPosition);
        float4 viewPosition = mul(ViewMatrix, worldPosition);
        float4 clipPosition = mul(ProjectionMatrix, viewPosition);
        
        verts[gtid].position = clipPosition;
        verts[gtid].color = float4(1.0f, 1.0f, 1.0f, 1.0f);
        verts[gtid].uv = float2(0.0, 0.0);
        verts[gtid].normal = float4(0.0f, 0.0f, 0.0f, 0.0f);
    }

    if (gtid < GRASS_PRIM_COUNT)
    {
        tris[gtid] = GrassBladePrim[gtid];
    }
}

[earlydepthstencil]
float4 PixelMain(m2p_t input) : SV_Target0
{
    float4 modelColor = ModelColor;
    float4 color = float4(0.0, 1.0, 0.0, 1.0) * modelColor;
    clip(color.a - 0.01f);
    
    return color;
}