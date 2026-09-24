
struct Camera {
    float4 position;
    float4 direction;
    uint gridHeight;
    uint gridWidth;
    uint gridDepth;
    uint padding;
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
    float2 ndc = float2((uv.x - 0.5) * 2.0, -(uv.y - 0.5) * 2.0);

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


static float3 lightDir = normalize(float3(1,1,1));

uint GetVoxel(int x, int y, int z) {
        int mappedY = y;
        int mappedZ = z;
        int mappedX = x;



        if (x< constants.gridWidth && y < constants.gridHeight && z < constants.gridDepth) {
            return voxels[constants.gridHeight*constants.gridDepth*mappedX + constants.gridDepth*mappedY  + mappedZ];
        }
        return 0;
   }


//stole this from the fast voxel traversal algorithm paper
Ray GetNextRay(Ray r,out float3 normal){
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
        normal = float3(-stepX,0,0);
    }else if(tMaxY < tMaxZ){
        r.voxelCoord.y += stepY;
        r.origin += r.direction * tMaxY;
        normal = float3(0,-stepY,0);
    }else{
      r.voxelCoord.z += stepZ;
      r.origin += r.direction * tMaxZ;
        normal = float3(0,0,-stepZ);
    }

    return r;
}

struct HitResult{
    int3 index;
    float3 hitPoint;
    float3 normal;
    uint material;
};

bool DoesRayIntersectBounds(Ray r,out Ray rayWithinBounds){


    float3 boxMin = float3 (0.0,0.0,0.0);
    float3 boxMax = float3(constants.gridWidth,constants.gridHeight,constants.gridDepth);

    float3 invDir = 1.0/ r.direction;
    float3 t0 = (boxMin - r.origin) * invDir;
    float3 t1 = (boxMax - r.origin) * invDir;

    float3 tmin = min(t0,t1);
    float3 tmax = max(t0,t1);

    float tnear = max(max(tmin.x,tmin.y), tmin.z);
    float tfar = min(min(tmax.x,tmax.y),tmax.z);

    if(tnear > tfar || tfar < 0.0){
        return false;
    }

    if(tnear > 0.0){
        rayWithinBounds.origin = r.origin;
        rayWithinBounds.direction = r.direction;

        rayWithinBounds.origin+= r.direction * (tnear + 0.0001);
        rayWithinBounds.voxelCoord = GetVoxelCoordinates(rayWithinBounds.origin);
        return true;
    }
    rayWithinBounds = r;

    return true;


}

HitResult Traverse(Ray cam){



    HitResult res;
    res.index = -1;
    res.material = 0;
    res.normal = float3(0,1,0);

Ray adjustedRay {};
  bool inBounds = DoesRayIntersectBounds(cam,adjustedRay);

    if(!inBounds){
        return res;
    }

cam = adjustedRay;
    while(true){
        if(cam.voxelCoord .x < 0 || cam.voxelCoord.y < 0 || cam.voxelCoord.z < 0){
            return res;
        }
        if(cam.voxelCoord.x >= constants.gridWidth || cam.voxelCoord.y >= constants.gridHeight || cam.voxelCoord.z >= constants.gridDepth){
            return res;
        }

        uint voxel = GetVoxel(cam.voxelCoord.x, cam.voxelCoord.y, cam.voxelCoord.z);
        if(voxel > 0){
            res.index = cam.voxelCoord;
            res.material = voxel;
            res.hitPoint = cam.origin.xyz;

            return res;
        }
        cam = GetNextRay(cam,res.normal);
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

float4 GetPixelColor(Ray r) {
    HitResult res = Traverse(r);
    //cast a ray on the light direction


    Ray reflectRay;
    reflectRay.origin = res.hitPoint + res.normal * 0.001;
    reflectRay.direction = lightDir;
    reflectRay.voxelCoord = GetVoxelCoordinates (reflectRay.origin);

    HitResult reflectRes = Traverse(reflectRay);
    if(reflectRes.material <1){
        //ray escaped
        return GetColor(res.material);
    }

    return GetColor(res.material) * .2f;
}


[numthreads(8, 8, 1)] void
CSMain(uint3 dispatchThreadID : SV_DispatchThreadID) {

  float uvx = (float)dispatchThreadID.x / 1000.0;
  float uvy = (float)dispatchThreadID.y / 1000.0;

  float2 uv = float2(uvx, uvy);

  Ray r = CreateCameraRay(uv);

  //HitResult res = Traverse(r);

  float4 c = GetPixelColor( r);

  OutputTexture[dispatchThreadID.xy] = float4(c.xyz,  1.0);
}
