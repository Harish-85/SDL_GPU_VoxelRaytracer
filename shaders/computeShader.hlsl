
struct Camera {
    float4 position;


    float4 direction;

};

ConstantBuffer<Camera> constants : register(b0,space2);
RWTexture2D<float4> OutputTexture : register(u0,space1);



[numthreads(8, 8, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float r = (float)dispatchThreadID.x/500.0;
    float g = (float)dispatchThreadID.y/500.0;

    float2 uv = float2(r,g);


    float2 ndc = float2((uv.x - 0.5) * 2.0, (uv.y - 0.5) * 2.0);

    float3 worldUp = float3(0.0, 1.0, 0.0);


    float3 camForward = normalize(constants.direction.xyz);
    float3 camRight   = normalize(cross(worldUp, camForward));
    float3 camUp      = cross(camForward, camRight);



    float fovScale = 1.0;
    float3 rayDirection = normalize(camForward + (ndc.x * camRight * fovScale) + (ndc.y * camUp * fovScale));




    OutputTexture[dispatchThreadID.xy] = float4(rayDirection * 0.5 + 0.5, 1.0);

}
