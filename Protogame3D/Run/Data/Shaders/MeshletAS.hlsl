#include "MeshletUtils.hlsli"

// The groupshared payload data to export to dispatched mesh shader threadgroups
groupshared Payload s_Payload;

// FRUSTUM CULLING---------------------------------------------------------

bool IsFrustumCulled(CullData c, float4x4 modelMatrix, float scale)
{
    float4 center = mul(modelMatrix, float4(c.BoundingSphere.xyz, 1));
    float radius = c.BoundingSphere.w * scale;
    
    for (int i = 0; i < 6; ++i)
    {
        if (dot(center.xyz, Planes[i].xyz) - Planes[i].w < -radius)
        {
            return true;
        }
    }
    
    return false;
}

//-------------------------------------------------------------------------

// BACKFACE CULLING--------------------------------------------------------

bool IsBackfaceCulled(CullData c, float4x4 modelMatrix, float scale, float3 cullViewPosition)
{
    if (IsConeDegenerate(c))
        return false;
        
    float4 center = mul(modelMatrix, float4(c.BoundingSphere.xyz, 1));
    
		// Unpack normal cone from uint8
    float4 normalCone = UnpackCone(c.NormalCone);
	
        // Transform axis to world space
    float3 axis = normalize(mul(modelMatrix, float4(normalCone.xyz, 0))).xyz;

        // Offset the normal cone axis from the meshlet center-point - make sure to account for world scaling
    float3 apex = center.xyz - axis * c.ApexOffset * scale;
    float3 view = normalize(cullViewPosition - apex);

        // The normal cone w-component stores -cos(angle + 90 deg)
        // This is the min dot product along the inverted axis from which all the meshlet's triangles are backface
    if (dot(view, -axis) > normalCone.w)
    {
        return true;
    }
    
    return false;
}

//-------------------------------------------------------------------------

// OCCLUSION CULLING-------------------------------------------------------

bool IsOcclusionCulled(CullData c, float4x4 modelMatrix, float scale, float3 cullViewPosition)
{
    float4 center = mul(modelMatrix, float4(c.BoundingSphere.xyz, 1));
    float radius = c.BoundingSphere.w * scale;
    
    // Get sphere min Z
    float4 sphereViewPosition = mul(CullingViewMatrix, float4(center.xyz, 1.f));
    float sphereMinZ = sphereViewPosition.x - radius;
        
        // Early out, if near plane clips sphere then just make it visible
    if (sphereMinZ <= 0.1f)
    {
        return false;
    }
        
        // Get screen space bounding box from bounding sphere
    AABB2 aabb = ComputeScreenSpaceAABB(sphereViewPosition.xyz, radius);
		
		// Compute the appropriate mip to lookup
    uint mipWidth, mipHeight;
    hiZMips[0].GetDimensions(mipWidth, mipHeight);
        
    float2 boxDimensions = float2(aabb.maxs - aabb.mins);
    float2 pixelDimensions = boxDimensions * float2(mipWidth, mipHeight);
    float maxDimension = max(max(pixelDimensions.x, pixelDimensions.y) / 1.f, 1.f);
        
    int mipLevel = floor(log2(maxDimension));
        
        // Get 2 remaining box corners
    float2 topLeftBoxCorner = aabb.mins + float2(0.f, boxDimensions.y);
    float2 bottomRightBoxCorner = aabb.mins + float2(boxDimensions.x, 0.f);
		
    hiZMips[mipLevel].GetDimensions(mipWidth, mipHeight);
        
    float maxXCoord = float(mipWidth);
    float maxYCoord = float(mipHeight);
    int2 topLeftLookupCoords = int2(maxXCoord * topLeftBoxCorner.x, maxYCoord * (1.f - topLeftBoxCorner.y));
    int2 bottomLeftLookupCoords = int2(maxXCoord * aabb.mins.x, maxYCoord * (1.f - aabb.mins.y));
    int2 bottomRightLookupCoords = int2(maxXCoord * bottomRightBoxCorner.x, maxYCoord * (1.f - bottomRightBoxCorner.y));
    int2 topRightLookupCoords = int2(maxXCoord * aabb.maxs.x, maxYCoord * (1.f - aabb.maxs.y));

        // Load max depths
    float topLeftMaxDepth = hiZMips[mipLevel].Load(topLeftLookupCoords);
    float bottomLeftMaxDepth = hiZMips[mipLevel].Load(bottomLeftLookupCoords);
    float bottomRightMaxDepth = hiZMips[mipLevel].Load(bottomRightLookupCoords);
    float topRightMaxDepth = hiZMips[mipLevel].Load(topRightLookupCoords);
        
        // Check if we're sampling outside the mip texture bounds, draw the meshlet if so since it's partially in view
    if (topLeftMaxDepth == 0.f || bottomLeftMaxDepth == 0.f || bottomRightMaxDepth == 0.f || topRightMaxDepth == 0.f)
    {
        return false;
    }
        
        // Project sphere min Z 
    float4 sphereMinClipPos = mul(CullingProjectionMatrix, float4(sphereMinZ, sphereViewPosition.yz, 1.f));
    float sphereMinDepth = sphereMinClipPos.z / sphereMinClipPos.w;
        
        // Cull if meshlet is behind all occluders in 4 texels
    if ((sphereMinDepth > topLeftMaxDepth) && (sphereMinDepth > bottomLeftMaxDepth) && (sphereMinDepth > bottomRightMaxDepth) && (sphereMinDepth > topRightMaxDepth))
    {
        return true;
    }
    
    return false;
}

//-------------------------------------------------------------------------

[numthreads(128, 1, 1)]
void AmpMain( 
        uint gtid : SV_GroupThreadID,
        uint dtid : SV_DispatchThreadID, 
        uint gid : SV_GroupID)
{
    bool isFrustumCulled        = false;
    bool isBackfaceCulled       = false;
    bool isOcclusionCulled      = false;
    bool visible                = false;
    bool shouldDraw             = false;

    uint meshletIndex           = gid / InstanceCount;
    uint instanceIndex          = gid % InstanceCount;

    // Check bounds of meshlet cull data resource
    if (gtid < MeshletCount * InstanceCount)
    {   
        if (isSecondPass)
        {
            if (frustumCulling)
            {
                isFrustumCulled = IsFrustumCulled(MeshletCullData[meshletIndex], InstanceData[instanceIndex].instanceTransform, InstanceData[instanceIndex].instanceScale);
            }

            if (backfaceCulling)
            {
                isBackfaceCulled = IsBackfaceCulled(MeshletCullData[meshletIndex], InstanceData[instanceIndex].instanceTransform, InstanceData[instanceIndex].instanceScale, CullViewPosition);
            }
            
            visible = !isFrustumCulled && !isBackfaceCulled;
            
            if (visible)
            {
                if (occlusionCulling)
                {
                    isOcclusionCulled = IsOcclusionCulled(MeshletCullData[meshletIndex], InstanceData[instanceIndex].instanceTransform, InstanceData[instanceIndex].instanceScale, CullViewPosition);
                }

                visible = visible && !isOcclusionCulled;
            }
            
            int currentIndex = instanceIndex + (meshletIndex * InstanceCount);
            
            bool wasVisibleLastFrame = LastPassVisibilities[currentIndex] == 1;
            shouldDraw = visible && !wasVisibleLastFrame;
            
            LastPassVisibilities[currentIndex] = visible == true ? 1 : 0;
        }
        else
        {
            int currentIndex = instanceIndex + (meshletIndex * InstanceCount);

            visible = LastPassVisibilities[currentIndex] == 1 ? true : false;
            
            bool lastVisibility = LastPassVisibilities[currentIndex] == 1 ? true : false;
            
            if (visible)
            {
                if (frustumCulling)
                {
                    isFrustumCulled = IsFrustumCulled(MeshletCullData[meshletIndex], InstanceData[instanceIndex].instanceTransform, InstanceData[instanceIndex].instanceScale);
                }

                if (backfaceCulling)
                {
                    isBackfaceCulled = IsBackfaceCulled(MeshletCullData[meshletIndex], InstanceData[instanceIndex].instanceTransform, InstanceData[instanceIndex].instanceScale, CullViewPosition);
                }
                
                visible = visible && !isFrustumCulled && !isBackfaceCulled;
            }
            
            shouldDraw = visible && lastVisibility;
        }
        
        //if (frustumCulling)
        //{
        //    isFrustumCulled = IsFrustumCulled(MeshletCullData[meshletIndex], InstanceData[instanceIndex].instanceTransform, InstanceData[instanceIndex].instanceScale);
        //}

        //if (backfaceCulling)
        //{
        //    isBackfaceCulled = IsBackfaceCulled(MeshletCullData[meshletIndex], InstanceData[instanceIndex].instanceTransform, InstanceData[instanceIndex].instanceScale, CullViewPosition);
        //}
            
        //visible = !isFrustumCulled && !isBackfaceCulled;
        
        //shouldDraw = visible;

        //if(isDepthPrePass)
        //{
        //    if (isSecondPass)
        //    {
        //        if (visible)
        //        {
        //            if (occlusionCulling)
        //            {
        //                isOcclusionCulled = IsOcclusionCulled(MeshletCullData[meshletIndex], InstanceData[instanceIndex].instanceTransform, InstanceData[instanceIndex].instanceScale, CullViewPosition);
        //            }

        //            visible = visible && !isOcclusionCulled;
        //        }
        //    }
            
        //    shouldDraw = visible;
        //}
        //else
        //{
        //    int currentIndex = instanceIndex + (meshletIndex * InstanceCount);
        
        //    if (isSecondPass)
        //    {
        //        if (visible)
        //        {
        //            if (occlusionCulling)
        //            {
        //                isOcclusionCulled = IsOcclusionCulled(MeshletCullData[meshletIndex], InstanceData[instanceIndex].instanceTransform, InstanceData[instanceIndex].instanceScale, CullViewPosition);
        //            }

        //            visible = visible && !isOcclusionCulled;
        //        }
            
        //        bool wasVisibleLastFrame = LastPassVisibilities[currentIndex] == 1;
        //        shouldDraw = visible && !wasVisibleLastFrame;
            
        //        LastPassVisibilities[currentIndex] = visible == true ? 1 : 0;
        //    }
        //    else
        //    {
        //        bool lastVisibility = LastPassVisibilities[currentIndex] == 1 ? true : false;

        //        shouldDraw = visible && lastVisibility;
        //    }
        //}
        
        //if (frustumCulling)
        //{
        //     // Per meshlet frustum culling
        //    isFrustumCulled = IsFrustumCulled(MeshletCullData[meshletIndex], InstanceData[instanceIndex].instanceTransform, InstanceData[instanceIndex].instanceScale);
        //}
        
        //if (backfaceCulling)
        //{
        //    // Per meshlet backface culling
        //    isBackfaceCulled = IsBackfaceCulled(MeshletCullData[meshletIndex], InstanceData[instanceIndex].instanceTransform, InstanceData[instanceIndex].instanceScale, CullViewPosition);
        //}
        
        //// Second pass in frame
        //if (SecondRenderPass)
        //{
        //    if (occlusionCulling)
        //    {
        //        // Do visibility testing for this thread
        //        isOcclusionCulled = IsOcclusionCulled(MeshletCullData[meshletIndex], InstanceData[instanceIndex].instanceTransform, InstanceData[instanceIndex].instanceScale, CullViewPosition);
        //    }
        //}
        
        //// Is meshlet visible after all culling checks?
        //visible = !isFrustumCulled && !isBackfaceCulled && !isOcclusionCulled;
            }
    
    if (visible)
    {
        InterlockedAdd(RealTimeData[0].DrawnVertexCount, Meshlets[dtid].vertexCount * InstanceCount);
        InterlockedAdd(RealTimeData[0].DrawnTriangleCount, Meshlets[dtid].primitiveCount * InstanceCount);
    }
    
    // Compact visible meshlets into the export payload array
    if (shouldDraw)
    {
        uint index = WavePrefixCountBits(shouldDraw);
        s_Payload.MeshletIndices[index] = meshletIndex;
        s_Payload.InstanceIndices[index] = instanceIndex;
    }
    
	// Dispatch mesh shaders required for visible meshlets, and output payload
    uint visibleMeshletCount = WaveActiveCountBits(visible);
    uint drawnMeshletCount = WaveActiveCountBits(shouldDraw);
    
    // Write info back to CPU
    if (gtid == 0)
    {
        InterlockedAdd(RealTimeData[0].DrawnMeshletCount, visibleMeshletCount);
    }
    
    DispatchMesh(drawnMeshletCount, 1, 1, s_Payload);
}