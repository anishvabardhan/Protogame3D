#define THREADS_PER_WAVE 128
#define AS_GROUP_SIZE THREADS_PER_WAVE

#define NUM_OF_HI_Z_MIPS 8

struct ms_input_t
{
    float3 localPosition;
    float4 color;
    float2 uv;
    float3 localTangent;
    float3 localBitangent;
    float3 localNormal;
};

struct m2p_t
{
	float4 position : SV_Position;
	float4 color : COLOR;
    float2 uv : TEXCOORD;
    float4 tangent : TANGENT;
    float4 bitangent : BITANGENT;
    float4 normal : NORMAL;
    float4 worldPosition : WORLDPOSITION;
};

struct RealtimeData
{
    uint DrawnMeshletCount;
    uint CulledMeshletCount;
    uint DrawnVertexCount;
    uint DrawnTriangleCount;
};

struct Meshlet
{
    uint vertexOffset;
    uint vertexCount;

    uint primitiveOffset;
    uint primitiveCount;
    float4 color;
};

struct MeshletInstance
{
    float4x4 instanceTransform;
    float instanceScale;
};

struct CullData
{
    float4 BoundingSphere;
    uint NormalCone;
    float ApexOffset;
};

struct Payload
{
    uint MeshletIndices[AS_GROUP_SIZE];
    uint InstanceIndices[AS_GROUP_SIZE];
};

cbuffer CameraConstants : register(b0)
{
    float4x4 ViewMatrix;
    float4x4 ProjectionMatrix;
};

cbuffer ModelConstants : register(b1)
{
    float4 ModelColor;
    float4x4 ModelMatrix;
};

cbuffer Profiler : register(b2)
{
    int meshletView;
    int frustumCulling;
    int backfaceCulling;
    int distanceCulling;
    int occlusionCulling;
    int isFogOn;
    int normalView;
};

cbuffer MeshInfo : register(b4)
{
    uint MeshletCount;
};

cbuffer Instance : register(b5)
{
    int InstanceCount;
};

cbuffer FrustumConstants : register(b6)
{
    float4 Planes[6];
    float3 CullViewPosition;
};

cbuffer DirectionalLight : register(b7)
{
    float3 SunDirection;
    float SunIntensity;
    float AmbientIntensity;
    float pad0;
    float pad1;
    float pad2;
};

cbuffer Constants : register(b8)
{
    float4x4 CullingViewMatrix;
    float4x4 CullingProjectionMatrix;
}

cbuffer SecondPass : register(b9)
{
    int isSecondPass;
};

StructuredBuffer<Meshlet>           Meshlets                    : register(t1);
StructuredBuffer<ms_input_t>        Vertices                    : register(t2);
StructuredBuffer<uint>              UniqueVertexIndices         : register(t3);
StructuredBuffer<uint3>             UniquePrimitiveIndices      : register(t4);
StructuredBuffer<CullData>          MeshletCullData             : register(t5);
StructuredBuffer<MeshletInstance>   InstanceData                : register(t6);

RWTexture2D<float>                  hiZMips[NUM_OF_HI_Z_MIPS]   : register(u0);
RWStructuredBuffer<uint>            LastPassVisibilities        : register(u11);
RWStructuredBuffer<RealtimeData>    RealTimeData                : register(u12);

bool IsConeDegenerate(CullData c)
{
    return (c.NormalCone >> 24) == 0xff;
}

float4 UnpackCone(uint packed)
{
    float4 v;
    v.x = float((packed >> 0) & 0xFF);
    v.y = float((packed >> 8) & 0xFF);
    v.z = float((packed >> 16) & 0xFF);
    v.w = float((packed >> 24) & 0xFF);

    v = v / 255.0;
    v.xyz = v.xyz * 2.0 - 1.0;

    return v;
}

struct AABB2
{
    float2 mins;
    float2 maxs;
};

AABB2 ComputeScreenSpaceAABB(float3 sphereViewCenter, float sphereRadius)
{
    float3 cr = sphereViewCenter * sphereRadius;
    float sphereMinDistanceSquared = sphereViewCenter.x * sphereViewCenter.x - sphereRadius * sphereRadius;

    float vy = sqrt(sphereViewCenter.y * sphereViewCenter.y + sphereMinDistanceSquared);
    float maxX = (vy * sphereViewCenter.y - cr.x) / (vy * sphereViewCenter.x + cr.y);
    float minX = (vy * sphereViewCenter.y + cr.x) / (vy * sphereViewCenter.x - cr.y);

    float vz = sqrt(sphereViewCenter.z * sphereViewCenter.z + sphereMinDistanceSquared);
    float maxY = (vz * sphereViewCenter.z - cr.x) / (vz * sphereViewCenter.x + cr.z);
    float minY = (vz * sphereViewCenter.z + cr.x) / (vz * sphereViewCenter.x - cr.z);

    float p00 = CullingProjectionMatrix[0][1];
    float p11 = CullingProjectionMatrix[1][2];
    minX *= p00; // P[0][0]
    minY *= p11; // P[1][1]
    maxX *= p00;
    maxY *= p11;

    minX *= 0.5f;
    minY *= 0.5f;
    maxX *= 0.5f;
    maxY *= 0.5f;

    minX += 0.5f;
    minY += 0.5f;
    maxX += 0.5f;
    maxY += 0.5f;

    AABB2 bounds;
    bounds.mins = saturate(float2(minX, maxY));
    bounds.maxs = saturate(float2(maxX, minY));
    return bounds;
}

float GetDistanceSquared(float3 a, float3 b)
{
    float distX = a.x - b.x;
    float distY = a.y - b.y;
    float distZ = a.z - b.z;

    float distance = (distX * distX) + (distY * distY) + (distZ * distZ);

    return distance;
}