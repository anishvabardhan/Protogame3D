SamplerState depthSampler : register(s0);

Texture2D<float> srcBuffer : register(t0);

RWTexture2D<float> dstBuffer : register(u0);

[numthreads(16, 16, 1)]
void ComputeMain(uint3 index : SV_DispatchThreadID, uint3 gtid : SV_GroupThreadID, uint3 gid : SV_GroupID, uint dtid : SV_DispatchThreadID)
{    
    int2 mipTexelCoords = index.xy;
            
    // Gather 4 texels from lower mip
    float northWestDepth = srcBuffer.Load(int3((mipTexelCoords * 2) + int2(0, 0), 0));
    float northEastDepth = srcBuffer.Load(int3((mipTexelCoords * 2) + int2(1, 0), 0));
    float southWestDepth = srcBuffer.Load(int3((mipTexelCoords * 2) + int2(0, 1), 0));
    float southEastDepth = srcBuffer.Load(int3((mipTexelCoords * 2) + int2(1, 1), 0));
            
    // Max depth of 4 previous mip texels
    float maxDepth = max(northWestDepth, max(northEastDepth, max(southWestDepth, southEastDepth)));
            
    // Check if we need more texels if mip dimensions are odd
    uint sourceMipWidth, sourceMipHeight;
    srcBuffer.GetDimensions(sourceMipWidth, sourceMipHeight);
    
    bool shouldGetExtraColumn = ((sourceMipWidth & 1) != 0);
    bool shouldGetExtraRow = ((sourceMipHeight & 1) != 0);
            
    // Check for extra column
    if (shouldGetExtraColumn)
    {
        float columnXDepth = srcBuffer.Load(int3((mipTexelCoords * 2) + int2(2, 0), 0));
        float columnYDepth = srcBuffer.Load(int3((mipTexelCoords * 2) + int2(2, 1), 0));
                
        // if both, need to include a corner value
        if (shouldGetExtraRow)
        {
            float cornerDepth = srcBuffer.Load(int3((mipTexelCoords * 2) + int2(2, 2), 0));
                    
            maxDepth = max(maxDepth, cornerDepth);
        }
                
        maxDepth = max(maxDepth, max(columnXDepth, columnYDepth));
    }
    // Check for extra row
    if (shouldGetExtraRow)
    {
        float rowXDepth = srcBuffer.Load(int3((mipTexelCoords * 2) + int2(0, 2), 0));
        float rowYDepth = srcBuffer.Load(int3((mipTexelCoords * 2) + int2(1, 2), 0));
                
        maxDepth = max(maxDepth, max(rowXDepth, rowYDepth));
    }
            
    // Store max depth in this mip texel            
    dstBuffer[mipTexelCoords] = maxDepth;
}