#ifndef HEIGHTMAP_H
#define HEIGHTMAP_H

struct HeightmapData {
    unsigned char* data;
    int width;
    int height;
    int channels;
};

HeightmapData loadHeightmap(const char* filename);
float getHeight(const HeightmapData& heightmap, int x, int z);
void freeHeightmap(HeightmapData& heightmap);

#endif
