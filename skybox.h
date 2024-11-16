#ifndef SKYBOX_H
#define SKYBOX_H

#include <vector>
#include <GL/glew.h>

class Skybox {
public:
    static std::vector<float> frontWallVertices;
    static std::vector<float> backWallVertices;
    static std::vector<float> leftWallVertices;
    static std::vector<float> rightWallVertices;
    static std::vector<float> roofVertices;
    static std::vector<float> floorVertices;

    static void updateCubeVertices(float size) {
        float halfSize = size / 2.0f;

        // Front wall vertices
        float frontWall[] = {
            -halfSize, 0.0f,  halfSize,
             halfSize, 0.0f,  halfSize,
             halfSize,  halfSize,  halfSize,
            -halfSize,  halfSize,  halfSize
        };

        // Back wall vertices
        float backWall[] = {
            -halfSize, 0.0f, -halfSize,
            -halfSize,  halfSize, -halfSize,
             halfSize,  halfSize, -halfSize,
             halfSize, 0.0f, -halfSize
        };

        // Left wall vertices
        float leftWall[] = {
            -halfSize, 0.0f, -halfSize,
            -halfSize, 0.0f,  halfSize,
            -halfSize,  halfSize,  halfSize,
            -halfSize,  halfSize, -halfSize
        };

        // Right wall vertices
        float rightWall[] = {
            halfSize, 0.0f, -halfSize,
            halfSize,  halfSize, -halfSize,
            halfSize,  halfSize,  halfSize,
            halfSize, 0.0f,  halfSize
        };

        // Roof vertices
        float roof[] = {
            -halfSize, halfSize, -halfSize,
            -halfSize, halfSize,  halfSize,
             halfSize, halfSize,  halfSize,
             halfSize, halfSize, -halfSize
        };

        // Floor vertices
        float floor[] = {
            -halfSize, 0.0f, -halfSize,
             halfSize, 0.0f, -halfSize,
             halfSize, 0.0f,  halfSize,
            -halfSize, 0.0f,  halfSize
        };

        // Clear and update all vertex arrays
        updateVertexArray(frontWallVertices, frontWall, sizeof(frontWall));
        updateVertexArray(backWallVertices, backWall, sizeof(backWall));
        updateVertexArray(leftWallVertices, leftWall, sizeof(leftWall));
        updateVertexArray(rightWallVertices, rightWall, sizeof(rightWall));
        updateVertexArray(roofVertices, roof, sizeof(roof));
        updateVertexArray(floorVertices, floor, sizeof(floor));
    }

private:
    static void updateVertexArray(std::vector<float>& target, float* source, size_t size) {
        target.clear();
        for (size_t i = 0; i < size / sizeof(float); i++) {
            target.push_back(source[i]);
        }
    }
};

// Define the static member
// std::vector<float> Skybox::globalCubeVertices;
std::vector<float> Skybox::frontWallVertices;
std::vector<float> Skybox::backWallVertices;
std::vector<float> Skybox::leftWallVertices;
std::vector<float> Skybox::rightWallVertices;
std::vector<float> Skybox::roofVertices;
std::vector<float> Skybox::floorVertices;

#endif // SKYBOX_H