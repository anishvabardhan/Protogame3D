#include "MeshletUtils.hlsli"

Texture2D       srcTexture          : register(t0);
Texture2D       normalMap           : register(t7);
SamplerState    diffuseSampler      : register(s0);

float4 TransformPosition(float x, float y, float z, int instanceIndex)
{
    float4 localPosition = float4(x, y, z, 1);

    float4 worldPosition = mul(InstanceData[instanceIndex].instanceTransform, localPosition);
    float4 viewPosition = mul(ViewMatrix, worldPosition);
    float4 clipPosition = mul(ProjectionMatrix, viewPosition);
	
    return clipPosition;
}

float4 TransformPositionToWorldSpace(float x, float y, float z, int instanceIndex)
{
    float4 localPosition = float4(x, y, z, 1);

    float4 worldPosition = mul(InstanceData[instanceIndex].instanceTransform, localPosition);
	
    return worldPosition;
}

// Mesh Shader

[outputtopology("triangle")]
[numthreads(64, 1, 1)]
void MeshMain(
        uint                            dtid : SV_DispatchThreadID,
        uint                            gid : SV_GroupID,
        uint                            gtid : SV_GroupIndex,
        in payload      Payload         s_Payload,
        out vertices    m2p_t           outVerts[64], 
        out indices     uint3           outIndices[42] )
{   
    uint meshletIndex = s_Payload.MeshletIndices[gid];
    uint instanceIndex = s_Payload.InstanceIndices[gid];
    
    if (meshletIndex >= MeshletCount * InstanceCount)
        return;
        
    Meshlet meshlet = Meshlets[meshletIndex];
    
    SetMeshOutputCounts(meshlet.vertexCount * InstanceCount, meshlet.primitiveCount * InstanceCount);
    
    if (gtid < meshlet.vertexCount * InstanceCount)
    {
        uint vi = UniqueVertexIndices[meshlet.vertexOffset + gtid];
        
        ms_input_t vertex = Vertices[vi];
        
        float4 clipPos = TransformPosition(vertex.localPosition.x, vertex.localPosition.y, vertex.localPosition.z, instanceIndex);
                
        float4 localNormal = float4(vertex.localNormal, 0);
        float4 worldNormal = mul(ModelMatrix, localNormal);
        float4 localTangent = float4(vertex.localTangent, 0);
        float4 worldTangent = mul(ModelMatrix, localTangent);
        float4 localBiTangent = float4(vertex.localBitangent, 0);
        float4 worldBiTangent = mul(ModelMatrix, localBiTangent);
        
        outVerts[gtid].position = clipPos;
        outVerts[gtid].color = meshletView == 0 ? vertex.color : meshlet.color;
        outVerts[gtid].uv = vertex.uv;
        outVerts[gtid].normal = worldNormal;
        outVerts[gtid].tangent = worldTangent;
        outVerts[gtid].bitangent = worldBiTangent;
        outVerts[gtid].worldPosition = TransformPositionToWorldSpace(vertex.localPosition.x, vertex.localPosition.y, vertex.localPosition.z, instanceIndex);

    }
        
    if (gtid < meshlet.primitiveCount * InstanceCount)
    {
        outIndices[gtid] = UniquePrimitiveIndices[meshlet.primitiveOffset + gtid];
    }
}

// Pixel Shader

[earlydepthstencil]
float4 PixelMain(m2p_t input) : SV_Target0
{   
    float4 color;
    
    float ambient = AmbientIntensity;
    
    float3 worldNormal = normalize(input.normal.xyz);
    float3 pixelWorldNormal = worldNormal;
    
    if (!meshletView)
    {
        float3 tangentNormal = 2.0f * normalMap.Sample(diffuseSampler, input.uv).rgb - 1.0f;
        float3x3 tbnMat = float3x3(normalize(input.tangent.xyz), normalize(input.bitangent.xyz), normalize(input.normal.xyz));
        pixelWorldNormal = mul(tangentNormal, tbnMat);
    }
    
    float dotProd = dot(worldNormal, -SunDirection);
    
    float falloff = clamp(dotProd, 0.0f, 0.1f);
    float falloffT = (falloff - 0.0f) / (0.1f - 0.0f);
    float falloffMultiplier = lerp(0.0f, 1.0f, falloffT);
    
    float diffuse = SunIntensity * falloffMultiplier * saturate(dot(normalize(pixelWorldNormal), -SunDirection));
   
    float4 lightColor = float4((ambient + diffuse).xxx, 1);
    float4 textureColor = srcTexture.Sample(diffuseSampler, input.uv);
    float4 vertexColor = input.color;
    float4 modelColor = ModelColor;
    
    color = vertexColor * modelColor;
    
    if (!meshletView && !normalView)
    {
        color *= textureColor * lightColor;
    }
    else if(!meshletView && normalView)
    {
        color = float4(input.normal.xyz * 0.5 + float3(0.5, 0.5, 0.5), 1.0);

    }
    
    clip(color.a - 0.01f);
    
    if (isFogOn)
    {
        // FOG EFFECT
    
        float3 dispCamToPixel = input.worldPosition.xyz - CullViewPosition.xyz;
        float distCamToPixel = length(dispCamToPixel);
        float fogDensity = 1.0f * saturate((distCamToPixel - 100.0f) / (200.0f - 100.0f));
        float3 finalRGB = lerp(color.xyz, float3(0.784f, 0.902f, 1.0f), fogDensity);
        float finalAlpha = saturate(textureColor.a + fogDensity); // fog can add opacity
        float4 finalColor = float4(finalRGB, finalAlpha);
        return finalColor;
    }
    else
    {
        return color;
    }
}