// Pyramid vertices and colors
// Vertex and color arrays for the pyramid
float pyramidVertices[] = {
    // Front face
    0.0f, 1.0f, 0.0f,  -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f, //triangle face
    // Right face
    0.0f, 1.0f, 0.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f, //triangle face
    // Back face
    0.0f, 1.0f, 0.0f,  1.0f, -1.0f, -1.0f,  -1.0f, -1.0f, -1.0f, //triangle face
    // Left face
    0.0f, 1.0f, 0.0f,  -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f, 1.0f, //triangle face
    // Bottom face
    -1.0f, -1.0f, 1.0f,   1.0f, -1.0f, 1.0f,   1.0f, -1.0f, -1.0f,  -1.0f, -1.0f, -1.0f //bottom square
};
float pyramidColors[] = {
    // Front face (red, green, blue)
    1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
    // Right face (red, blue, green)
    1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f,
    // Back face (red, green, blue)
    1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
    // Left face (red, blue, green)
    1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f,
    // Bottom face (raindbow)
    1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f
};
