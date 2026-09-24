#include <iostream>

#include "MagicaVoxelLoader.h"
#include "SDLFullScreenComputeRenderer.h"
#include "VoxelData.h"

int main() {

    //MagicaVoxelDataProvider voxelDataProvier("/home/harish/Downloads/#treehouse.vox");
    MagicaVoxelDataProvider voxelDataProvier("assets/voxelFiles/#street_scene.vox");

    voxelDataProvier.LoadVoxelAsset();

    auto voxeldata = voxelDataProvier.GetVoxelData();

    SDLFullScreenComputeRenderer fullscreenRenderer(voxeldata.Voxels,voxelDataProvier.GetColors(),voxeldata.BoundsWidth,voxeldata.BoundsHeight,voxeldata.BoundsDepth);


}
