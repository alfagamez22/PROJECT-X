// Pyramid vertices and colors
float scaling = 40.0f;
// Vertex and color arrays for the pyramid
float pyramidVertices[] = {
    // Front face
    0.0f, scaling * 1.0f, 0.0f,  -scaling * 1.0f, -scaling * 1.0f, scaling * 1.0f,  scaling * 1.0f, -scaling * 1.0f, scaling * 1.0f,

    // Right face
    0.0f, scaling * 1.0f, 0.0f,  scaling * 1.0f, -scaling * 1.0f, scaling * 1.0f,  scaling * 1.0f, -scaling * 1.0f, -scaling * 1.0f,

    // Back face
    0.0f, scaling * 1.0f, 0.0f,  scaling * 1.0f, -scaling * 1.0f, -scaling * 1.0f,  -scaling * 1.0f, -scaling * 1.0f, -scaling * 1.0f,

    // Left face
    0.0f, scaling * 1.0f, 0.0f,  -scaling * 1.0f, -scaling * 1.0f, -scaling * 1.0f,  -scaling * 1.0f, -scaling * 1.0f, scaling * 1.0f,

    // Bottom face
    -scaling * 1.0f, -scaling * 1.0f,  scaling * 1.0f,
     scaling * 1.0f, -scaling * 1.0f,  scaling * 1.0f,
     scaling * 1.0f, -scaling * 1.0f, -scaling * 1.0f,
    -scaling * 1.0f, -scaling * 1.0f, -scaling * 1.0f
};

float pyramidTextureCoords[] = {
    // Front face
    0.5f, 1.0f,    0.0f, 0.0f,    1.0f, 0.0f,  // Triangle
    // Right face
    0.5f, 1.0f,    0.0f, 0.0f,    1.0f, 0.0f,  // Triangle
    // Back face
    0.5f, 1.0f,    0.0f, 0.0f,    1.0f, 0.0f,  // Triangle
    // Left face
    0.5f, 1.0f,    0.0f, 0.0f,    1.0f, 0.0f,  // Triangle
    // Bottom face
    0.0f, 0.0f,    1.0f, 0.0f,    1.0f, 1.0f,    0.0f, 1.0f  // Square
};