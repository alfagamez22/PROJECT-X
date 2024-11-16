#ifndef SKYBOX_H
#define SKYBOX_H

#include <vector>
#include <GL/glew.h>

class Skybox {
public:
    static std::vector<float> globalCubeVertices;

    static void updateCubeVertices(float size) {
        float halfSize = size / 2.0f;
        float vertices[] = {
            // Front face OR WALL
            -halfSize, 0.0f,  halfSize,
             halfSize, 0.0f,  halfSize,
             halfSize,  halfSize,  halfSize,
            -halfSize,  halfSize,  halfSize,
            // Back face OR WALL
            -halfSize, 0.0f, -halfSize,
            -halfSize,  halfSize, -halfSize,
             halfSize,  halfSize, -halfSize,
             halfSize, 0.0f, -halfSize,
             // Left face OR WALL
             -halfSize, 0.0f, -halfSize,
             -halfSize, 0.0f,  halfSize,
             -halfSize,  halfSize,  halfSize,
             -halfSize,  halfSize, -halfSize,

             // Right face OR WALL
             halfSize, 0.0f, -halfSize,
             halfSize,  halfSize, -halfSize,
             halfSize,  halfSize,  halfSize,
             halfSize, 0.0f,  halfSize,

             // ROOF OR SKY
             -halfSize, halfSize, -halfSize,
             -halfSize, halfSize,  halfSize,
              halfSize, halfSize,  halfSize,
              halfSize, halfSize, -halfSize,

              //FLOOR
              -halfSize, 0.0f, -halfSize,
               halfSize, 0.0f, -halfSize,
               halfSize, 0.0f, halfSize,
              -halfSize, 0.0f, halfSize
        };

        globalCubeVertices.clear();
        for (int i = 0; i < sizeof(vertices) / sizeof(float); i++) {
            globalCubeVertices.push_back(vertices[i]);
        }
    }
};

// Define the static member
std::vector<float> Skybox::globalCubeVertices;

#endif // SKYBOX_H