//
// Created by harish on 9/12/26.
//

#ifndef VOXEL_RAYTRACER_VOXELDATA_H
#define VOXEL_RAYTRACER_VOXELDATA_H
#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <vector>



class VoxelData {
public:
    unsigned const int BoundsHeight,BoundsWidth,BoundsDepth;
    std::vector<uint32_t> Voxels;

    VoxelData(const int height, const int width, const int depth,std::vector<uint32_t> voxels) : BoundsHeight(height), BoundsWidth(width),
                                                                        BoundsDepth(depth) {
        Voxels = voxels;
    }

    uint32_t GetVoxel(int x, int y, int z) {
        int mappedY = y;
        int mappedZ = z;
        int mappedX = x;

        if (x< BoundsWidth && y < BoundsHeight && z < BoundsDepth) {
            return Voxels[BoundsHeight*BoundsDepth*mappedX + BoundsDepth*mappedY  + mappedZ];
        }
        return -1;
    }
};


class IVoxelDataProvider {
public:

    virtual VoxelData GetVoxelData() = 0;
    virtual void LoadVoxelAsset() = 0;
};

struct MVVoxel {
    int8_t x,y,z;
    uint8_t color;
};

struct Color {
    int8_t r,g,b,a;
};





#endif //VOXEL_RAYTRACER_VOXELDATA_H