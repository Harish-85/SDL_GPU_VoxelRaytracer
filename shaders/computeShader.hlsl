
struct Camera {
    float4 position;


    float4 direction;

};

struct Voxel{
    uint material;
};

ConstantBuffer<Camera> constants : register(b0,space2);
RWTexture2D<float4> OutputTexture : register(u0,space1);
StructuredBuffer<uint> voxels :register(t0,space0);


[numthreads(8, 8, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint testVoxel = voxels[1];

    // 2. Output a stark color based on whether it is populated
    if (testVoxel > 0)
    {
        // If the voxel has a material ID, paint a solid green overlay in the corner
        if (dispatchThreadID.x < 50 && dispatchThreadID.y < 50) {
            OutputTexture[dispatchThreadID.xy] = float4(0.0, 1.0, 0.0, 1.0); // Solid Green
            return;
        }
    }
    else
    {
        // If it evaluates to 0 (or wasn't uploaded), paint a solid red overlay instead
        if (dispatchThreadID.x < 50 && dispatchThreadID.y < 50) {
            OutputTexture[dispatchThreadID.xy] = float4(1.0, 0.0, 0.0, 1.0); // Solid Red
            return;
        }
    }

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

    //trace this



    OutputTexture[dispatchThreadID.xy] = float4(rayDirection * 0.5 + 0.5, 1.0);

}
