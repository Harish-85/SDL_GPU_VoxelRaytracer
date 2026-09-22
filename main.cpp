#include <iostream>

#include "SDLFullScreenComputeRenderer.h"
#include "VoxelData.h"

int main() {

    MagicaVoxelDataProvider voxelDataProvier("/home/harish/Downloads/#treehouse.vox");
    voxelDataProvier.LoadVoxelAsset();

    SDLFullScreenComputeRenderer fullscreenRenderer(voxelDataProvier.GetVoxelData().Voxels);


}
