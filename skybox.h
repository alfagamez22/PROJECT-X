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
    static std::vector<float> subDividedFloorVertices; // New vector for subdivided floor

    // Function to subdivide floor into a grid
    static void subdivideFloor(int resolution, float size) {
        subDividedFloorVertices.clear();
        float halfSize = size / 2.0f;
        float step = size / static_cast<float>(resolution);

        // Generate grid vertices
        for (int i = 0; i <= resolution; ++i) {
            for (int j = 0; j <= resolution; ++j) {
                float x = -halfSize + j * step;
                float z = -halfSize + i * step;

                // Add vertex position (x, y, z)
                subDividedFloorVertices.push_back(x);
                subDividedFloorVertices.push_back(0.0f); // y is always 0 for floor
                subDividedFloorVertices.push_back(z);
            }
        }
    }
     // Add after subdivideFloor method
    static void applyHeightmap(float amplitude, float frequency) {
        for (size_t i = 0; i < subDividedFloorVertices.size(); i += 3) {
            float x = subDividedFloorVertices[i];
            float z = subDividedFloorVertices[i + 2];

            // Generate height using sine waves for a desert dune effect
            float height = amplitude * (
                sin(x * frequency) * cos(z * frequency) +
                sin(x * frequency * 0.5f) * cos(z * frequency * 0.5f)
                );

            // Update Y coordinate (height)
            subDividedFloorVertices[i + 1] = height;
        }
    }

    // Generate indices for the subdivided floor (if needed for rendering)
    static std::vector<unsigned int> generateFloorIndices(int resolution) {
        std::vector<unsigned int> indices;
        for (int i = 0; i < resolution; ++i) {
            for (int j = 0; j < resolution; ++j) {
                unsigned int topLeft = i * (resolution + 1) + j;
                unsigned int topRight = topLeft + 1;
                unsigned int bottomLeft = (i + 1) * (resolution + 1) + j;
                unsigned int bottomRight = bottomLeft + 1;

                // First triangle
                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                // Second triangle
                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }
        return indices;
    }

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

        // Subdivide the floor with a default resolution (can be changed later)
        subdivideFloor(10, size); // Default 10x10 grid
    }

private:
    static void updateVertexArray(std::vector<float>& target, float* source, size_t size) {
        target.clear();
        for (size_t i = 0; i < size / sizeof(float); i++) {
            target.push_back(source[i]);
        }
    }
};

// Define the static members
std::vector<float> Skybox::frontWallVertices;
std::vector<float> Skybox::backWallVertices;
std::vector<float> Skybox::leftWallVertices;
std::vector<float> Skybox::rightWallVertices;
std::vector<float> Skybox::roofVertices;
std::vector<float> Skybox::floorVertices;
std::vector<float> Skybox::subDividedFloorVertices;

#endif // SKYBOX_H


// #ifndef SKYBOX_H
// #define SKYBOX_H

// #include <vector>
// #include <GL/glew.h>

// class Skybox {
// public:
//     static std::vector<float> frontWallVertices;
//     static std::vector<float> backWallVertices;
//     static std::vector<float> leftWallVertices;
//     static std::vector<float> rightWallVertices;
//     static std::vector<float> roofVertices;
//     static std::vector<float> floorVertices;

//     static void updateCubeVertices(float size) {
//         float halfSize = size / 2.0f;

//         // Front wall vertices
//         float frontWall[] = {
//             -halfSize, 0.0f,  halfSize,
//              halfSize, 0.0f,  halfSize,
//              halfSize,  halfSize,  halfSize,
//             -halfSize,  halfSize,  halfSize
//         };

//         // Back wall vertices
//         float backWall[] = {
//             -halfSize, 0.0f, -halfSize,
//             -halfSize,  halfSize, -halfSize,
//              halfSize,  halfSize, -halfSize,
//              halfSize, 0.0f, -halfSize
//         };

//         // Left wall vertices
//         float leftWall[] = {
//             -halfSize, 0.0f, -halfSize,
//             -halfSize, 0.0f,  halfSize,
//             -halfSize,  halfSize,  halfSize,
//             -halfSize,  halfSize, -halfSize
//         };

//         // Right wall vertices
//         float rightWall[] = {
//             halfSize, 0.0f, -halfSize,
//             halfSize,  halfSize, -halfSize,
//             halfSize,  halfSize,  halfSize,
//             halfSize, 0.0f,  halfSize
//         };

//         // Roof vertices
//         float roof[] = {
//             -halfSize, halfSize, -halfSize,
//             -halfSize, halfSize,  halfSize,
//              halfSize, halfSize,  halfSize,
//              halfSize, halfSize, -halfSize
//         };

//         // Floor vertices
//         float floor[] = {
//             -halfSize, 0.0f, -halfSize,
//              halfSize, 0.0f, -halfSize,
//              halfSize, 0.0f,  halfSize,
//             -halfSize, 0.0f,  halfSize
//         };

//         // Clear and update all vertex arrays
//         updateVertexArray(frontWallVertices, frontWall, sizeof(frontWall));
//         updateVertexArray(backWallVertices, backWall, sizeof(backWall));
//         updateVertexArray(leftWallVertices, leftWall, sizeof(leftWall));
//         updateVertexArray(rightWallVertices, rightWall, sizeof(rightWall));
//         updateVertexArray(roofVertices, roof, sizeof(roof));
//         updateVertexArray(floorVertices, floor, sizeof(floor));
//     }

// private:
//     static void updateVertexArray(std::vector<float>& target, float* source, size_t size) {
//         target.clear();
//         for (size_t i = 0; i < size / sizeof(float); i++) {
//             target.push_back(source[i]);
//         }
//     }
// };

// // Define the static member
// // std::vector<float> Skybox::globalCubeVertices;
// std::vector<float> Skybox::frontWallVertices;
// std::vector<float> Skybox::backWallVertices;
// std::vector<float> Skybox::leftWallVertices;
// std::vector<float> Skybox::rightWallVertices;
// std::vector<float> Skybox::roofVertices;
// std::vector<float> Skybox::floorVertices;

// #endif // SKYBOX_H