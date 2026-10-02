//
// Created by harish on 9/30/26.
//

#ifndef VOXEL_RAYTRACER_VOXELRENDEREROBJECT_H
#define VOXEL_RAYTRACER_VOXELRENDEREROBJECT_H
#include "ITickable.h"
#include "Object3D.h"
#include "VoxelData.h"

class VoxelRendererObject : public Object3D{

    VoxelData* _voxData;

public:
    VoxelRendererObject(IVoxelDataProvider voxel_data_provider) {
        _voxData = voxel_data_provider.GetVoxelData();
    }

    VoxelData* GetVoxelData() {

    }


};

#endif //VOXEL_RAYTRACER_VOXELRENDEREROBJECT_H