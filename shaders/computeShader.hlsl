
struct Camera {
    float4 position;
    float4 direction;
};

struct Voxel{
    uint material;
};

struct Color{
    uint r;
    uint g;
    uint b;
    uint a;
};

struct Ray{
    float3 origin;
    float3 direction;
    int3 voxelCoord;
};

ConstantBuffer<Camera> constants : register(b0,space2);
RWTexture2D<float4> OutputTexture : register(u0,space1);
StructuredBuffer<uint> voxels :register(t0,space0);
StructuredBuffer<uint> colors :register(t1,space0);

int3 GetVoxelCoordinates(float3 position)
{
    return int3(floor(position.x), floor(position.y), floor(position.z));
}
Ray CreateCameraRay(float2 uv){
    float2 ndc = float2((uv.x - 0.5) * 2.0, (uv.y - 0.5) * 2.0);

    float3 worldUp = float3(0.0, 1.0, 0.0);

    float3 camForward = normalize(constants.direction.xyz);
    float3 camRight   = normalize(cross(worldUp, camForward));
    float3 camUp      = cross(camForward, camRight);

    float fovScale = 1.0;

    Ray r;
    r.origin = constants.position.xyz;
    r.direction = normalize(camForward + (ndc.x * camRight * fovScale) + (ndc.y * camUp * fovScale));
    r.voxelCoord = GetVoxelCoordinates(constants.position.xyz);
    return r;
}


static int boundsWidth = 126;
static int boundsHeight=126;
static int boundsDepth=126;

uint GetVoxel(int x, int y, int z) {
        int mappedY = z;
        int mappedZ = boundsHeight - 1 -y;
        int mappedX = x;



        if (x< boundsWidth && y < boundsHeight && z < boundsDepth) {
            return voxels[boundsHeight*boundsDepth*mappedX + boundsDepth*mappedY  + mappedZ];
        }
        return 0;
   }


//stole this from the fast voxel traversal algorithm paper
Ray GetNextRay(Ray r){
    float tMaxX=0,tMaxY=0,tMaxZ=0;

    int stepX = (r.direction.x > 0) ? 1 : -1;
    int stepY = (r.direction.y > 0) ? 1 : -1;
    int stepZ = (r.direction.z > 0) ? 1 : -1;

    float tdx = abs( 1/r.direction.x);
    float tdy = abs( 1/r.direction.y);
    float tdz = abs( 1/r.direction.z);

    if(r.direction.x > 0){
        tMaxX = (r.voxelCoord.x + 1 - r.origin.x) * tdx;
    }else{
        tMaxX = (r.origin.x - r.voxelCoord.x) * tdx;
    }

    if(r.direction.y > 0){
        tMaxY = (r.voxelCoord.y + 1 - r.origin.y) * tdy;
    }else{
        tMaxY = (r.origin.y - r.voxelCoord.y) * tdy;
    }

    if(r.direction.z > 0){
        tMaxZ = (r.voxelCoord.z + 1 - r.origin.z) * tdz;
    }else{
      tMaxZ = (r.origin.z - r.voxelCoord.z) * tdz;
    }



    if(tMaxX < tMaxY && tMaxX < tMaxZ){
        r.voxelCoord.x += stepX;
        r.origin += r.direction * tMaxX;
    }else if(tMaxY < tMaxZ){
        r.voxelCoord.y += stepY;
        r.origin += r.direction * tMaxY;
    }else{
      r.voxelCoord.z += stepZ;
      r.origin += r.direction * tMaxZ;
    }

    return r;
}

uint Traverse(Ray cam){
    while(true){
        if(cam.voxelCoord .x < 0 || cam.voxelCoord.y < 0 || cam.voxelCoord.z < 0){
            return 0;
        }
        if(cam.voxelCoord.x > 126 || cam.voxelCoord.y > 126 || cam.voxelCoord.z > 126){
            return 0;
        }

        uint voxel = GetVoxel(cam.voxelCoord.x, cam.voxelCoord.y, cam.voxelCoord.z);
        if(voxel > 0){
            return voxel;
        }
        cam = GetNextRay(cam);
    }
}

float4 GetColor(uint index) {
    uint packed_val = colors[index];

    float r = float((packed_val >>  0u) & 0xFFu) / 255.0;
    float g = float((packed_val >>  8u) & 0xFFu) / 255.0;
    float b = float((packed_val >> 16u) & 0xFFu) / 255.0;
    float a = float((packed_val >> 24u) & 0xFFu) / 255.0;

    return float4(r, g, b, a);
}

[numthreads(8, 8, 1)] void
CSMain(uint3 dispatchThreadID : SV_DispatchThreadID) {
  uint testVoxel = voxels[1];

  // 2. Output a stark color based on whether it is populated
  if (testVoxel > 0) {
    // If the voxel has a material ID, paint a solid green overlay in the corner
    if (dispatchThreadID.x < 50 && dispatchThreadID.y < 50) {
      OutputTexture[dispatchThreadID.xy] =
          float4(0.0, 1.0, 0.0, 1.0); // Solid Green
      return;
    }
  } else {
    // If it evaluates to 0 (or wasn't uploaded), paint a solid red overlay
    // instead
    if (dispatchThreadID.x < 50 && dispatchThreadID.y < 50) {
      OutputTexture[dispatchThreadID.xy] =
          float4(1.0, 0.0, 0.0, 1.0); // Solid Red
      return;
    }
  }

  float uvx = (float)dispatchThreadID.x / 500.0;
  float uvy = (float)dispatchThreadID.y / 500.0;

  float2 uv = float2(uvx, uvy);

  Ray r = CreateCameraRay(uv);

  uint res = Traverse(r);

  float4 c = GetColor(res);

  OutputTexture[dispatchThreadID.xy] = float4(c.rgb, 1.0);
}
