//
// Created by harish on 9/24/26.
//

#ifndef VOXEL_RAYTRACER_MAGICAVOXELLOADER_H
#define VOXEL_RAYTRACER_MAGICAVOXELLOADER_H
#include <cstdint>

#include "VoxelData.h"

struct MVVoxel;

class MagicaVoxelData {
public:
    char Header[4];
    int32_t Version;
    int BoundsHeight,BoundsWidth,BoundsDepth;
    std::vector<MVVoxel> Voxels;
    Color Colors[256];

    void SetBounds(int h,int w,int d) {
        BoundsHeight = h;
        BoundsWidth = w;
        BoundsDepth = d;


    }

    void AddVoxel(const int8_t x, const int8_t y, const int8_t z, const uint8_t color ) {
        Voxels.push_back( MVVoxel{ x ,y,z,color});
    }

    void AddColor(const int index,const int8_t r,const int8_t g, const int8_t b, const int8_t a) {
        Colors[index] =  Color{ r,g,b,a};
    }
};

class MagicaVoxelDataProvider : IVoxelDataProvider {
private:
    std::string _path;
    std::ifstream readFile;

    MagicaVoxelData data;


    static constexpr uint32_t GetIntFrom4Chars(const char chunkId[4] ) {
        const uint32_t res =  static_cast<uint32_t>(chunkId[0]) |
            static_cast<uint32_t>(chunkId[1]) << 8 |
                static_cast<uint32_t>(chunkId[2]) << 16 |
                    static_cast<uint32_t>(chunkId[3]) << 24;

        return res;
    }


public:
    MagicaVoxelDataProvider(const std::string &path) {
        _path = path;
    }

    ~MagicaVoxelDataProvider() {
        readFile.close();
    }

    Color* GetColors() {
        return data.Colors;
    }

    void LoadVoxelAsset() {
        readFile = std::ifstream(_path,std::ios_base::binary);


        if (!readFile.is_open()) {
            std::cerr<<"Failed ot open file at " + _path;
            return;
        }

        readFile.seekg(0, std::ios::end);
        std::streampos fileSize = readFile.tellg();
        readFile.seekg(0, std::ios::beg);

        std::cout<<"Opened magica voxel file" <<std::endl;

        //read first 4 bytes
        readFile.read( data.Header, 4);
        readFile.read(reinterpret_cast<char*>(&data.Version), 4);

        std::cout.write(data.Header, 4);
        std::cout <<"version " << data.Version <<std::endl;


        while (true) {

            int32_t startG = readFile.tellg();
            //var current chunk info
            char chunkId[4];
            readFile.read( chunkId,4);

            uint32_t chunkSize = 0;
            uint32_t childrenSize = 0;

            readFile.read( reinterpret_cast<char *>(&chunkSize),4);
            readFile.read( reinterpret_cast<char *>(&childrenSize),4);


            std::cout.write(chunkId, 4);
            std::cout<<std::endl;
            std::cout <<"ChunkSize " << chunkSize <<std::endl;
            std::cout << "Children Size " <<childrenSize<<std::endl;

            switch (GetIntFrom4Chars(chunkId)) {
                case GetIntFrom4Chars("MAIN"):
                    std::cout << "MAIN Chunk detected ... doing nothing" <<std::endl;
                    break;
                case GetIntFrom4Chars("SIZE"):
                    std::cout << "SIZE Chunk detected "<<std::endl;

                    int width,height,depth;

                    readFile.read( reinterpret_cast<char *>(&width),4);
                    readFile.read( reinterpret_cast<char *>(&depth),4);
                    readFile.read( reinterpret_cast<char *>(&height),4);

                    data.SetBounds( height,width,depth);

                    break;
                case GetIntFrom4Chars("XYZI"):
                    std::cout << "XYZI Chunk detected"<<std::endl;
                    int32_t numVoxels;
                    readFile.read( reinterpret_cast<char *>(&numVoxels),4);


                    for (int i =0; i< numVoxels ; i++) {
                        int8_t x,y,z,color;

                        readFile.read( reinterpret_cast<char *>(&x),1);
                        readFile.read( reinterpret_cast<char *>(&y),1);
                        readFile.read( reinterpret_cast<char *>(&z),1);
                        readFile.read( reinterpret_cast<char *>(&color),1);

                        data.AddVoxel( x,y,z,color);
                    }


                    break;

                case GetIntFrom4Chars("RGBA"):
                    std::cout << "RGBA Chunk detected " <<std::endl;
                    for (int i =0; i< 255 ; i++) {
                        int8_t r,g,b,a;

                        readFile.read( reinterpret_cast<char *>(&r),1);
                        readFile.read( reinterpret_cast<char *>(&g),1);
                        readFile.read( reinterpret_cast<char *>(&b),1);
                        readFile.read( reinterpret_cast<char *>(&a),1);

                        data.AddColor(i+1, r,g,b,a);
                    }

                    break;
                default:
                    std::cout<<"Attempting to execute a invalid chink id "<<std::endl;
                    break;

            }

            readFile.seekg(startG + chunkSize + 12); //12 for the header
            if (readFile.tellg() >= fileSize) {
                std::cout << "File finished reading";
                break;
            }
        }


    }

    std::set<uint8_t> uniqueColors;
    VoxelData GetVoxelData() override {
        int width  = data.BoundsWidth;
        int height = data.BoundsDepth;
        int depth  = data.BoundsHeight;
        std::vector<uint32_t> voxels( data.BoundsHeight * data.BoundsWidth * data.BoundsDepth,0);

        std::set<uint8_t> nonZeroIndices;



        for (MVVoxel voxel: data.Voxels) {
            int adjX = voxel.x;
            int adjy =  voxel.z; // MV Z is Y here
            int adjz = voxel.y;

            int index = height* depth* adjX +
                depth*adjy  +adjz;
            voxels[index] = voxel.color;
            nonZeroIndices.insert(index);
        }

        int j =0;
        while (true) {
            if (voxels[j] == 0) {
                break;
            }
            j++;
        }
        std::cout<<"air voxel at " << j<<std::endl;
        VoxelData vData( height,width,depth,voxels);

        std::cout<<"Voxel bounds " << data.BoundsHeight << " " << data.BoundsWidth<< " " << data.BoundsHeight<<std::endl;

        return vData;
    }

};

#endif //VOXEL_RAYTRACER_MAGICAVOXELLOADER_H