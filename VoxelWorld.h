//
// Created by harish on 9/17/26.
//

#ifndef VOXEL_RAYTRACER_VOXELGRID_H
#define VOXEL_RAYTRACER_VOXELGRID_H
#include <cstdint>
#include <cstring>
#include <vector>
#include <glm/vec3.hpp>


const uint CHUNK_SIZE = 16;

struct GridChunk {
    uint8_t voxels[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE];
    bool isDirty;

    uint8_t GetVoxel(glm::ivec3 chunkIndex) {
        return voxels[CHUNK_SIZE * chunkIndex.z * chunkIndex.y + chunkIndex.y * CHUNK_SIZE + chunkIndex.x];
    }

    uint8_t SetVoxel(glm::ivec3 chunkIndex, uint8_t vox) {
        voxels[CHUNK_SIZE * CHUNK_SIZE * chunkIndex.z  + chunkIndex.y * CHUNK_SIZE + chunkIndex.x] = vox;
    }

    void ClearChunk() {

    }

};

class VoxelWorld {
    static const uint CHUNK_COUNT_WIDTH = 100,CHUNK_COUNT_HEIGHT=100,CHUNK_COUNT_DEPTH=100;
private:
    std::vector<uint32_t> topLevelGrid;
    std::vector<GridChunk> chunkPool;

    inline uint GetVolume() { return CHUNK_COUNT_DEPTH * CHUNK_COUNT_HEIGHT * CHUNK_COUNT_WIDTH;}

    void InitializeGrids() {
        topLevelGrid.resize(GetVolume() ,0);
        chunkPool.reserve( GetVolume()/2);
    }

    glm::ivec3 GetTopLevelGridIndex(int x,int y,int z) {
        glm::ivec3 tlIndex;
        tlIndex.x = x/CHUNK_SIZE;
        tlIndex.y = y/CHUNK_SIZE;
        tlIndex.z = z/CHUNK_SIZE;

        return tlIndex;
    }

    glm::ivec3 GetGridIndexInChunk(glm::ivec3 index) {
        glm::ivec3 chunkIndex;
        chunkIndex.x = index.x%CHUNK_SIZE;
        chunkIndex.y = index.y%CHUNK_SIZE;
        chunkIndex.z = index.z%CHUNK_SIZE;

        return chunkIndex;
    }

public:
    VoxelWorld() {
        InitializeGrids();
    }

    void WriteVoxel(int x, int y, int z, uint8_t vox) {
        glm::ivec3 tIndex = GetTopLevelGridIndex(x,y,z);
        uint32_t chunkIndex = GetChunkIndex(tIndex);


        if (topLevelGrid[chunkIndex] == 0) {
            chunkPool.emplace_back();
            topLevelGrid[chunkIndex] = chunkPool.size()-1;
        }

        chunkPool[ topLevelGrid[chunkIndex] ].SetVoxel( GetGridIndexInChunk(glm::ivec3(x,y,z)),vox);
    }

    uint8_t GetVoxel(int x, int y ,int z) {
        glm::ivec3 tIndex = GetTopLevelGridIndex(x,y,z);
        uint32_t chunkIndex = GetChunkIndex(tIndex);


        if (topLevelGrid[chunkIndex] == 0) {
            return 0;
        }

        return chunkPool[ topLevelGrid[chunkIndex] ].GetVoxel( GetGridIndexInChunk(glm::ivec3(x,y,z)));
    }

    uint32_t GetChunkIndex(glm::ivec3 chunkIndex) {
        return CHUNK_COUNT_HEIGHT * CHUNK_COUNT_DEPTH * chunkIndex.x + CHUNK_COUNT_DEPTH * chunkIndex.y + chunkIndex.z;
    }

};


#endif //VOXEL_RAYTRACER_VOXELGRID_H