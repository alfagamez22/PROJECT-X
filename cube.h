const float CUBE_SCALE = 1.0f; // Adjust this to change cube size



// Vertex and texture arrays for the cube
float cubeVertices[] = {
    // Top face
    CUBE_SCALE * 1.0f, CUBE_SCALE * 1.0f, CUBE_SCALE * -1.0f,  CUBE_SCALE * -1.0f, CUBE_SCALE * 1.0f, CUBE_SCALE * -1.0f,  CUBE_SCALE * -1.0f, CUBE_SCALE * 1.0f, CUBE_SCALE * 1.0f,  CUBE_SCALE * 1.0f, CUBE_SCALE * 1.0f, CUBE_SCALE * 1.0f,
    // Bottom face
    CUBE_SCALE * 1.0f, CUBE_SCALE * -1.0f, CUBE_SCALE * 1.0f,  CUBE_SCALE * -1.0f, CUBE_SCALE * -1.0f, CUBE_SCALE * 1.0f,  CUBE_SCALE * -1.0f, CUBE_SCALE * -1.0f, CUBE_SCALE * -1.0f,  CUBE_SCALE * 1.0f, CUBE_SCALE * -1.0f, CUBE_SCALE * -1.0f,
    // Front face
    CUBE_SCALE * 1.0f, CUBE_SCALE * 1.0f, CUBE_SCALE * 1.0f,  CUBE_SCALE * -1.0f, CUBE_SCALE * 1.0f, CUBE_SCALE * 1.0f,  CUBE_SCALE * -1.0f, CUBE_SCALE * -1.0f, CUBE_SCALE * 1.0f,  CUBE_SCALE * 1.0f, CUBE_SCALE * -1.0f, CUBE_SCALE * 1.0f,
    // Back face
    CUBE_SCALE * 1.0f, CUBE_SCALE * -1.0f, CUBE_SCALE * -1.0f,  CUBE_SCALE * -1.0f, CUBE_SCALE * -1.0f, CUBE_SCALE * -1.0f,  CUBE_SCALE * -1.0f, CUBE_SCALE * 1.0f, CUBE_SCALE * -1.0f,  CUBE_SCALE * 1.0f, CUBE_SCALE * 1.0f, CUBE_SCALE * -1.0f,
    // Left face
    CUBE_SCALE * -1.0f, CUBE_SCALE * 1.0f, CUBE_SCALE * 1.0f,  CUBE_SCALE * -1.0f, CUBE_SCALE * 1.0f, CUBE_SCALE * -1.0f,  CUBE_SCALE * -1.0f, CUBE_SCALE * -1.0f, CUBE_SCALE * -1.0f,  CUBE_SCALE * -1.0f, CUBE_SCALE * -1.0f, CUBE_SCALE * 1.0f,
    // Right face
    CUBE_SCALE * 1.0f, CUBE_SCALE * 1.0f, CUBE_SCALE * -1.0f,  CUBE_SCALE * 1.0f, CUBE_SCALE * 1.0f, CUBE_SCALE * 1.0f,  CUBE_SCALE * 1.0f, CUBE_SCALE * -1.0f, CUBE_SCALE * 1.0f,  CUBE_SCALE * 1.0f, CUBE_SCALE * -1.0f, CUBE_SCALE * -1.0f
};

float textureCoords[] = {
    // Front face
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    // Back face
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    // Top face
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    // Bottom face
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
    // Right face
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
    0.0f, 0.0f,
    // Left face
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f
};