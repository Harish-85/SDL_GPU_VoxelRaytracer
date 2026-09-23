#include <iostream>

#include "SDLFullScreenComputeRenderer.h"
#include "VoxelData.h"

int main() {

    MagicaVoxelDataProvider voxelDataProvier("/home/harish/Downloads/#treehouse.vox");

    voxelDataProvier.LoadVoxelAsset();

    auto voxeldata = voxelDataProvier.GetVoxelData();

    SDLFullScreenComputeRenderer fullscreenRenderer(voxeldata.Voxels,voxelDataProvier.GetColors());


}
