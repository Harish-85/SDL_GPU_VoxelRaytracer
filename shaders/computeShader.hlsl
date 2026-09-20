
[[vk::image_format("rgba8")]]
RWTexture2D<float4> OutputTexture : register(u0,space1);

[numthreads(8, 8, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float r = (float)dispatchThreadID.x/500.0;
    float g = (float)dispatchThreadID.y/500.0;

    OutputTexture[dispatchThreadID.xy] = float4(r,g,0.5,1.0);
}
