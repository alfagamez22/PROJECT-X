#define GLEW_STATIC
#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"
//#include "TextureLoader.h"

#include <GL/glew.h>  // Added for VBO support
#include <GL/glut.h>
#include <GL/freeglut_ext.h>
#include <math.h>
#include <string.h>
#include <chrono>
#include <sstream>
#include <vector>

using namespace std;

// Function prototypes
void initGL();
void display();
void timer(int value);
void reshape(GLsizei width, GLsizei height);
void keyboardDown(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void mouseMovement(int x, int y);
void updateCameraDirection();
void updateMovement();
void renderPauseMenu();
void renderCube();
void renderPyramid();
void renderSphere3D(float radius, int segments, float angle, float xPos, float yPos, float zPos);
void renderAnotherSphere3D(float radius, int segments, float angle, float xPos, float yPos, float zPos);
void renderCubeMap();
void displayCoordinates(float coordX, float coordY, float coordZ);
void renderHUD();

// Function prototypes for VBOs
void initVBOs();

// Function prototypes for particle system
void spawnParticle();
void updateParticles();
void checkAnddespawnParticles();
void renderParticles();

/* Global variables */
float anglePyramid = 0.0f;
float angleCube = 0.0f;
float angleCircle = 0.0f;  // Rotation angle for the circle
int refreshMills = 5; // Refresh interval in milliseconds the lower the better maximum 5 minimum 25 (15 best) to avoid visual bugs

// Camera position and orientation
float cameraX = 0.0f;
float cameraY = 2.0f;
float cameraZ = 10.0f;
float cameraSpeed = 0.1f; //can be adjusted the higher the faster 0.1 is the best it was 0.3 kinda quick

// Movement flags and velocity
bool keyStates[256] = { false };
float velocityX = 0.0f;
float velocityZ = 0.0f;
float maxVelocity = 0.15f;
float acceleration = 0.05f;
float deceleration = 0.1f;

// Camera angles
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 320.0f;
float lastY = 240.0f;
float sensitivity = 0.1f;

// Camera direction vectors
float lookX = 0.0f;
float lookY = 0.0f;
float lookZ = -1.0f;

// Game state
bool isPaused = false;
bool firstMouse = true;

//Draw fps
chrono::time_point<chrono::high_resolution_clock> startTime;
int frameCount = 0;
float fps = 0.0f;

//Hud display
bool hudEnabled = false;

//Initialize VBOs
GLuint cubeVBOs[2], pyramidVBOs[2], sphere3DVBOs[2];

//number of segments for the sphere (higher is better) but may bug out 20 is best and smooth
const int segments = 20;

//particle system
const int maxParticles = 10; // Max particles in the system
//const float gravity = -9.81f; // Gravity constant (negative to pull downward) good for rain particle system for later
const float pi = M_PI;  // Using the irrational value of pi for randomness
float deltaT = 0.016f; // Time step (assuming 60 FPS)

//speed of the particles multiplier
float speedMultiplier = 1.01f;  // Adjust this to control particle speed (1.0 default speed kinda fast but good for desert scene)

// Particle struct and list to store particles
struct Particle {
    float position[3];
    float velocity[3];
    bool isActive;
};
std::vector<Particle> particles;


// Cube vertices and colors
 // Vertex and color arrays for the cube
float cubeVertices[] = {
    // Top face
    1.0f, 1.0f, -1.0f,  -1.0f, 1.0f, -1.0f,  -1.0f, 1.0f, 1.0f,  1.0f, 1.0f, 1.0f,
    // Bottom face
    1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
    // Front face
    1.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,
    // Back face
    1.0f, -1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f, 1.0f, -1.0f,  1.0f, 1.0f, -1.0f,
    // Left face
    -1.0f, 1.0f, 1.0f,  -1.0f, 1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f, 1.0f,
    // Right face
    1.0f, 1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f
};

float cubeColors[] = {
    // Top face (green)
    0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
    // Bottom face (orange)
    1.0f, 0.5f, 0.0f,  1.0f, 0.5f, 0.0f,  1.0f, 0.5f, 0.0f,  1.0f, 0.5f, 0.0f,
    // Front face (red)
    1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
    // Back face (yellow)
    1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,
    // Left face (blue)
    0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
    // Right face (magenta)
    1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f
};

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
    0.0f, 1.0f, 1.0f,  1.0f, 0.0f, 1.0f,  1.0f, 1.0f, 0.0f,
};



// Main display function
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("3D Shapes with First Person Controls");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutPassiveMotionFunc(mouseMovement);

    initGL();

    // Start the clock on startup from 0 to -> calculate FPS
    startTime = chrono::high_resolution_clock::now();

    // Initialize GLEW for OpenGL extension support
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW initialization failed: %s\n", glewGetErrorString(err));
        return 1;
    }

    initVBOs();

    glutTimerFunc(0, timer, 0);
    glutMainLoop();
    return 0;
}


void initGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glutSetCursor(GLUT_CURSOR_NONE);
}

void cubeVBO() {
    // Cube VBO setup
    glGenBuffers(2, cubeVBOs);

    // Cube vertex VBO
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // Cube color VBO
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeColors), cubeColors, GL_STATIC_DRAW);
}

void pyramidVBO() {
    // Pyramid VBO setup
    glGenBuffers(2, pyramidVBOs);

    // Pyramid vertex VBO
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

    // Pyramid color VBO
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidColors), pyramidColors, GL_STATIC_DRAW);
}

void sphere3dVBO_one() {
    //Sphere3D VBO setup
    glGenBuffers(2, sphere3DVBOs);

    // Generate sphere vertex data and color data before sending them to the VBO
    // const int segments = 20;  // You can adjust this for higher/lower resolution
    const int numVertices = (segments + 1) * (segments + 1);
    float sphereVertices[numVertices * 3];
    float sphereColors[numVertices * 3];
    int index = 0;

    for (int lat = 0; lat <= segments; ++lat) {
        float theta = lat * M_PI / segments; // Latitude angle
        for (int lon = 0; lon <= segments; ++lon) {
            float phi = lon * 2.0f * M_PI / segments; // Longitude angle

            // Calculate x, y, z positions
            sphereVertices[index * 3] = sin(theta) * cos(phi); // x
            sphereVertices[index * 3 + 1] = sin(theta) * sin(phi); // y
            sphereVertices[index * 3 + 2] = cos(theta); // z

            // Set color (gradient effect)
            sphereColors[index * 3] = (float)lat / segments; // Red
            sphereColors[index * 3 + 1] = (float)lon / segments; // Green
            sphereColors[index * 3 + 2] = 0.5f; // Blue

            ++index;
        }
    }
    // Sphere vertex VBO
    glBindBuffer(GL_ARRAY_BUFFER, sphere3DVBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphereVertices), sphereVertices, GL_STATIC_DRAW);

    // Sphere color VBO
    glBindBuffer(GL_ARRAY_BUFFER, sphere3DVBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sphereColors), sphereColors, GL_STATIC_DRAW);
}


void initVBOs() {

    cubeVBO();
    pyramidVBO();
    sphere3dVBO_one();

    // Unbind buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // // Cube VBO setup
    // glGenBuffers(2, cubeVBOs);

    // // Cube vertex VBO
    // glBindBuffer(GL_ARRAY_BUFFER, cubeVBOs[0]);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // // Cube color VBO
    // glBindBuffer(GL_ARRAY_BUFFER, cubeVBOs[1]);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(cubeColors), cubeColors, GL_STATIC_DRAW);

    // // Pyramid VBO setup
    // glGenBuffers(2, pyramidVBOs);

    // // Pyramid vertex VBO
    // glBindBuffer(GL_ARRAY_BUFFER, pyramidVBOs[0]);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

    // // Pyramid color VBO
    // glBindBuffer(GL_ARRAY_BUFFER, pyramidVBOs[1]);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidColors), pyramidColors, GL_STATIC_DRAW);

    // //Sphere3D VBO setup
    // glGenBuffers(2, sphere3DVBOs);

    // // Generate sphere vertex data and color data before sending them to the VBO
    // // const int segments = 20;  // You can adjust this for higher/lower resolution
    // const int numVertices = (segments + 1) * (segments + 1);
    // float sphereVertices[numVertices * 3];
    // float sphereColors[numVertices * 3];
    // int index = 0;

    // for (int lat = 0; lat <= segments; ++lat) {
    //     float theta = lat * M_PI / segments; // Latitude angle
    //     for (int lon = 0; lon <= segments; ++lon) {
    //         float phi = lon * 2.0f * M_PI / segments; // Longitude angle

    //         // Calculate x, y, z positions
    //         sphereVertices[index * 3] = sin(theta) * cos(phi); // x
    //         sphereVertices[index * 3 + 1] = sin(theta) * sin(phi); // y
    //         sphereVertices[index * 3 + 2] = cos(theta); // z

    //         // Set color (gradient effect)
    //         sphereColors[index * 3] = (float)lat / segments; // Red
    //         sphereColors[index * 3 + 1] = (float)lon / segments; // Green
    //         sphereColors[index * 3 + 2] = 0.5f; // Blue

    //         ++index;
    //     }
    // }
    // // Sphere vertex VBO
    // glBindBuffer(GL_ARRAY_BUFFER, sphere3DVBOs[0]);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(sphereVertices), sphereVertices, GL_STATIC_DRAW);

    // // Sphere color VBO
    // glBindBuffer(GL_ARRAY_BUFFER, sphere3DVBOs[1]);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(sphereColors), sphereColors, GL_STATIC_DRAW);

    // // Unbind buffer
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void updateCameraDirection() {
    lookX = cos(yaw * M_PI / 180.0f) * cos(pitch * M_PI / 180.0f);
    lookY = sin(pitch * M_PI / 180.0f);
    lookZ = sin(yaw * M_PI / 180.0f) * cos(pitch * M_PI / 180.0f);
}

void mouseMovement(int x, int y) {

    if (isPaused) return;
    if (firstMouse) {
        lastX = x;
        lastY = y;
        firstMouse = false;
        return;
    }

    // Get current window dimensions
    int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
    int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

    // Calculate window center
    int centerX = windowWidth / 2;
    int centerY = windowHeight / 2;

    float xoffset = x - lastX;
    float yoffset = lastY - y;

    // Only warp if we're near the edges (within 20% of window size)
    int edgeThreshold = windowWidth / 5;
    if (x <= edgeThreshold || x >= (windowWidth - edgeThreshold) ||
        y <= edgeThreshold || y >= (windowHeight - edgeThreshold)) {
        glutWarpPointer(centerX, centerY);
        lastX = centerX;
        lastY = centerY;
        return; // Skip processing this frame to prevent jitter
    }

    lastX = x;
    lastY = y;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Constrain pitch to prevent camera flipping from a full 360 degree rotation
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    updateCameraDirection();
}

void keyboardDown(unsigned char key, int x, int y) {
    keyStates[key] = true;

    if (key == 27) { // ESC key
        if (!isPaused) {
            isPaused = true;
            glutSetCursor(GLUT_CURSOR_INHERIT);
        }
        else {
            isPaused = false;
            glutSetCursor(GLUT_CURSOR_NONE);
        }
    }

    // Exit only if 'q' or 'Q' is pressed and the system is paused
    if (isPaused && (key == 'q' || key == 'Q')) {
        exit(0);
    }

    if (key == 'h' || key == 'H') {
        hudEnabled = !hudEnabled; //toggles hud visibility
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    keyStates[key] = false;
}

void updateMovement() {
    if (isPaused) return;

    float moveX = lookX;
    float moveZ = lookZ;

    // Normalize movement vector
    float length = sqrt(moveX * moveX + moveZ * moveZ);
    if (length > 0) {
        moveX /= length;
        moveZ /= length;
    }

    // Calculate desired velocity based on key states
    float targetVelocityX = 0.0f;
    float targetVelocityZ = 0.0f;

    if (keyStates['w'] || keyStates['W']) {
        targetVelocityX += moveX * maxVelocity;
        targetVelocityZ += moveZ * maxVelocity;
    }
    if (keyStates['s'] || keyStates['S']) {
        targetVelocityX -= moveX * maxVelocity;
        targetVelocityZ -= moveZ * maxVelocity;
    }
    if (keyStates['a'] || keyStates['A']) {
        targetVelocityX += moveZ * maxVelocity;
        targetVelocityZ -= moveX * maxVelocity;
    }
    if (keyStates['d'] || keyStates['D']) {
        targetVelocityX -= moveZ * maxVelocity;
        targetVelocityZ += moveX * maxVelocity;
    }

    // Smoothly interpolate current velocity towards target velocity
    velocityX = velocityX + (targetVelocityX - velocityX) * (targetVelocityX != 0 ? acceleration : deceleration);
    velocityZ = velocityZ + (targetVelocityZ - velocityZ) * (targetVelocityZ != 0 ? acceleration : deceleration);

    // Update camera position
    cameraX += velocityX;
    cameraZ += velocityZ;

    // Update Y position (instant, not smoothed)
    if (keyStates['q'] || keyStates['Q']) cameraY += cameraSpeed;
    if (keyStates['e'] || keyStates['E']) cameraY -= cameraSpeed;
}

void renderPauseMenu() {
    if (!isPaused) return;

    // Switch to orthographic projection for 2D rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT), 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Draw semi-transparent black overlay
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(glutGet(GLUT_WINDOW_WIDTH), 0);
    glVertex2f(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
    glVertex2f(0, glutGet(GLUT_WINDOW_HEIGHT));
    glEnd();
    glDisable(GL_BLEND);

    // Draw pause menu text
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2i(glutGet(GLUT_WINDOW_WIDTH) / 2 - 40, glutGet(GLUT_WINDOW_HEIGHT) / 2);
    const char* text = "\t \t \t MENU";
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    glRasterPos2i(glutGet(GLUT_WINDOW_WIDTH) / 2 - 80, glutGet(GLUT_WINDOW_HEIGHT) / 2 + 30);
    const char* subText = "Press ESC to resume or Q to quit";
    for (const char* c = subText; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }

    // Restore the projection matrix
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// void renderCube() {
    // // Vertex and color arrays for the cube
    // float cubeVertices[] = {
    //     // Top face
    //     1.0f, 1.0f, -1.0f,  -1.0f, 1.0f, -1.0f,  -1.0f, 1.0f, 1.0f,  1.0f, 1.0f, 1.0f,
    //     // Bottom face
    //     1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
    //     // Front face
    //     1.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,
    //     // Back face
    //     1.0f, -1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f, 1.0f, -1.0f,  1.0f, 1.0f, -1.0f,
    //     // Left face
    //     -1.0f, 1.0f, 1.0f,  -1.0f, 1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f, 1.0f,
    //     // Right face
    //     1.0f, 1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f
    // };

    // float cubeColors[] = {
    //     // Top face (green)
    //     0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 1.0f, 0.0f,
    //     // Bottom face (orange)
    //     1.0f, 0.5f, 0.0f,  1.0f, 0.5f, 0.0f,  1.0f, 0.5f, 0.0f,  1.0f, 0.5f, 0.0f,
    //     // Front face (red)
    //     1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
    //     // Back face (yellow)
    //     1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,
    //     // Left face (blue)
    //     0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
    //     // Right face (magenta)
    //     1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f,  1.0f, 0.0f, 1.0f
    // };

//     glPushMatrix();
//     glTranslatef(1.5f, 0.0f, -7.0f);
//     glRotatef(angleCube, 1.0f, 1.0f, 1.0f);

//     glEnableClientState(GL_VERTEX_ARRAY);
//     glEnableClientState(GL_COLOR_ARRAY);

//     glVertexPointer(3, GL_FLOAT, 0, cubeVertices);
//     glColorPointer(3, GL_FLOAT, 0, cubeColors);

//     glDrawArrays(GL_QUADS, 0, 24);  // 24 vertices, 6 faces

//     glDisableClientState(GL_VERTEX_ARRAY);
//     glDisableClientState(GL_COLOR_ARRAY);

//     glPopMatrix();
// }

// void renderPyramid() {
    // // Vertex and color arrays for the pyramid
    // float pyramidVertices[] = {
    //     // Front face
    //     0.0f, 1.0f, 0.0f,  -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,
    //     // Right face
    //     0.0f, 1.0f, 0.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f,
    //     // Back face
    //     0.0f, 1.0f, 0.0f,  1.0f, -1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,
    //     // Left face
    //     0.0f, 1.0f, 0.0f,  -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f, 1.0f,
    //     // Bottom face
    //     -1.0f, -1.0f, 1.0f,   1.0f, -1.0f, 1.0f,   1.0f, -1.0f, -1.0f,  -1.0f, -1.0f, -1.0f
    // };

    // float pyramidColors[] = {
    //     // Front face (red, green, blue)
    //     1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
    //     // Right face (red, blue, green)
    //     1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f,
    //     // Back face (red, green, blue)
    //     1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
    //     // Left face (red, blue, green)
    //     1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f, 0.0f,
    //     // Bottom face (raindbow)
    //     0.0f, 1.0f, 1.0f,  1.0f, 0.0f, 1.0f,  1.0f, 1.0f, 0.0f
    // };

//     glPushMatrix();
//     glTranslatef(-1.5f, 0.0f, -6.0f);
//     glRotatef(anglePyramid, 1.0f, 1.0f, 0.0f);

//     glEnableClientState(GL_VERTEX_ARRAY);
//     glEnableClientState(GL_COLOR_ARRAY);

//     glVertexPointer(3, GL_FLOAT, 0, pyramidVertices);
//     glColorPointer(3, GL_FLOAT, 0, pyramidColors);

//     glDrawArrays(GL_TRIANGLES, 0, 12);  // 12 vertices, 4 faces
//     glDrawArrays(GL_QUADS, 12, 4);   // 4 vertices, 1 face for the bottom face of the pyramid square

//     glDisableClientState(GL_VERTEX_ARRAY);
//     glDisableClientState(GL_COLOR_ARRAY);

//     glPopMatrix();

// }

void renderCube() {

    glPushMatrix();
    glTranslatef(1.5f, 0.0f, -7.0f);
    glRotatef(angleCube, 1.0f, 1.0f, 1.0f);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    // Bind vertex VBO
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBOs[0]);
    glVertexPointer(3, GL_FLOAT, 0, nullptr);

    // Bind color VBO
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBOs[1]);
    glColorPointer(3, GL_FLOAT, 0, nullptr);

    glDrawArrays(GL_QUADS, 0, 24);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO

    glPopMatrix();
}

void renderPyramid() {

    glPushMatrix();
    glTranslatef(-1.5f, 0.0f, -6.0f);
    glRotatef(anglePyramid, 1.0f, 1.0f, 0.0f);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    // Bind vertex VBO
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBOs[0]);
    glVertexPointer(3, GL_FLOAT, 0, nullptr);

    // Bind color VBO
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBOs[1]);
    glColorPointer(3, GL_FLOAT, 0, nullptr);

    glDrawArrays(GL_TRIANGLES, 0, 12);
    glDrawArrays(GL_QUADS, 12, 4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO

    glPopMatrix();
}


// void renderSphere3D(float radius, int segments, float angle, float xPos, float yPos, float zPos) {
//     // Arrays for vertices and colors
//     float sphereVertices[(segments + 1) * (segments + 1) * 3];
//     float sphereColors[(segments + 1) * (segments + 1) * 3];

//     int index = 0;
//     for (int lat = 0; lat <= segments; ++lat) {
//         float theta = lat * M_PI / segments;  // Latitude angle (from 0 to pi)
//         for (int lon = 0; lon <= segments; ++lon) {
//             float phi = lon * 2.0f * M_PI / segments; // Longitude angle (from 0 to 2pi)

//             // Calculate the x, y, z position of each vertex on the sphere
//             sphereVertices[index * 3] = radius * sin(theta) * cos(phi); // x
//             sphereVertices[index * 3 + 1] = radius * sin(theta) * sin(phi); // y
//             sphereVertices[index * 3 + 2] = radius * cos(theta); // z

//             // Set color for each vertex (simple gradient)
//             sphereColors[index * 3] = (float)lat / segments; // Red
//             sphereColors[index * 3 + 1] = (float)lon / segments; // Green
//             sphereColors[index * 3 + 2] = 0.5f; // Blue

//             ++index;
//         }
//     }

//     glPushMatrix();
//     // Position the sphere in 3D space using xPos, yPos, zPos
//     glTranslatef(xPos, yPos, zPos);

//     // Rotate the sphere for animation or effect
//     glRotatef(angle, 1.0f, 0.0f, 0.0f); // Rotate around X-axis
//     glRotatef(angle, 0.0f, 1.0f, 0.0f); // Rotate around Y-axis

//     glEnableClientState(GL_VERTEX_ARRAY);
//     glEnableClientState(GL_COLOR_ARRAY);

//     glVertexPointer(3, GL_FLOAT, 0, sphereVertices);
//     glColorPointer(3, GL_FLOAT, 0, sphereColors);

//     // Draw the solid sphere using GL_TRIANGLE_STRIP
//     for (int lat = 0; lat < segments; ++lat) {
//         for (int lon = 0; lon < segments; ++lon) {
//             // Get the 4 vertices of each quad (for each latitude-longitude pair)
//             int p1 = lat * (segments + 1) + lon;
//             int p2 = p1 + 1;
//             int p3 = (lat + 1) * (segments + 1) + lon;
//             int p4 = p3 + 1;

//             // Create two triangles for each quad
//             float vertices[6][3] = {
//                 {sphereVertices[p1 * 3], sphereVertices[p1 * 3 + 1], sphereVertices[p1 * 3 + 2]},
//                 {sphereVertices[p2 * 3], sphereVertices[p2 * 3 + 1], sphereVertices[p2 * 3 + 2]},
//                 {sphereVertices[p3 * 3], sphereVertices[p3 * 3 + 1], sphereVertices[p3 * 3 + 2]},
//                 {sphereVertices[p3 * 3], sphereVertices[p3 * 3 + 1], sphereVertices[p3 * 3 + 2]},
//                 {sphereVertices[p2 * 3], sphereVertices[p2 * 3 + 1], sphereVertices[p2 * 3 + 2]},
//                 {sphereVertices[p4 * 3], sphereVertices[p4 * 3 + 1], sphereVertices[p4 * 3 + 2]}
//             };

//             glBegin(GL_TRIANGLES);
//             for (int i = 0; i < 6; ++i) {
//                 glColor3fv(&sphereColors[(p1 + i) * 3]);
//                 glVertex3fv(vertices[i]);
//             }
//             glEnd();
//         }
//     }

//     glDisableClientState(GL_VERTEX_ARRAY);
//     glDisableClientState(GL_COLOR_ARRAY);

//     glPopMatrix();
// }


// continue this:
// void initVBOs() {
//     // Cube VBO setup
//     glGenBuffers(2, cubeVBOs);

//     // Cube vertex VBO
//     glBindBuffer(GL_ARRAY_BUFFER, cubeVBOs[0]);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

//     // Cube color VBO
//     glBindBuffer(GL_ARRAY_BUFFER, cubeVBOs[1]);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(cubeColors), cubeColors, GL_STATIC_DRAW);

//     // Pyramid VBO setup
//     glGenBuffers(2, pyramidVBOs);

//     // Pyramid vertex VBO
//     glBindBuffer(GL_ARRAY_BUFFER, pyramidVBOs[0]);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

//     // Pyramid color VBO
//     glBindBuffer(GL_ARRAY_BUFFER, pyramidVBOs[1]);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidColors), pyramidColors, GL_STATIC_DRAW);

//     //Sphere3D VBO setup
//     glGenBuffers(3, sphere3DVBOs);

//     //Sphere3D vertex VBO
//     glBindBuffer(GL_ARRAY_BUFFER, sphere3DVBOs[0]);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(sphere3DVertices), sphere3DVertices, GL_STATIC_DRAW);


//     // Unbind
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
// }

void renderSphere3D(float radius, int segments, float angle, float xPos, float yPos, float zPos) {
    glPushMatrix();
    glTranslatef(xPos, yPos, zPos);
    glScalef(radius, radius, radius);

    glRotatef(angle, 1.0f, 0.0f, 0.0f);
    glRotatef(angle, 0.0f, 1.0f, 0.0f);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    // Bind VBOs and set up pointers
    glBindBuffer(GL_ARRAY_BUFFER, sphere3DVBOs[0]);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, sphere3DVBOs[1]);
    glColorPointer(3, GL_FLOAT, 0, 0);

    // Render sphere
    for (int lat = 0; lat < segments; ++lat) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int lon = 0; lon <= segments; ++lon) {
            int p1 = lat * (segments + 1) + lon;
            int p2 = (lat + 1) * (segments + 1) + lon;

            glArrayElement(p1);
            glArrayElement(p2);
        }
        glEnd();
    }

    // Disable client states and unbind VBOs
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glPopMatrix();
}

void renderAnotherSphere3D(float radius, int segments, float angle, float xPos, float yPos, float zPos) {
    // Arrays for vertices and colors
    float sphereVertices[(segments + 1) * (segments + 1) * 3];
    float sphereColors[(segments + 1) * (segments + 1) * 3];

    int index = 0;
    for (int lat = 0; lat <= segments; ++lat) {
        float theta = lat * M_PI / segments;  // Latitude angle (from 0 to pi)
        for (int lon = 0; lon <= segments; ++lon) {
            float phi = lon * 2.0f * M_PI / segments; // Longitude angle (from 0 to 2pi)

            // Calculate the x, y, z position of each vertex on the sphere
            sphereVertices[index * 3] = radius * sin(theta) * cos(phi); // x
            sphereVertices[index * 3 + 1] = radius * sin(theta) * sin(phi); // y
            sphereVertices[index * 3 + 2] = radius * cos(theta); // z

            // Set color for each vertex (simple gradient)
            sphereColors[index * 3] = (float)lat / segments; // Red
            sphereColors[index * 3 + 1] = (float)lon / segments; // Green
            sphereColors[index * 3 + 2] = 0.5f; // Blue

            ++index;
        }
    }

    glPushMatrix();
    // Position the sphere in 3D space using xPos, yPos, zPos
    glTranslatef(xPos, yPos, zPos);

    // Rotate the sphere for animation or effect
    glRotatef(angle, 1.0f, 0.0f, 0.0f); // Rotate around X-axis
    glRotatef(angle, 0.0f, 1.0f, 0.0f); // Rotate around Y-axis

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, sphereVertices);
    glColorPointer(3, GL_FLOAT, 0, sphereColors);

    // Draw the solid sphere using GL_TRIANGLE_STRIP
    for (int lat = 0; lat < segments; ++lat) {
        for (int lon = 0; lon < segments; ++lon) {
            // Get the 4 vertices of each quad (for each latitude-longitude pair)
            int p1 = lat * (segments + 1) + lon;
            int p2 = p1 + 1;
            int p3 = (lat + 1) * (segments + 1) + lon;
            int p4 = p3 + 1;

            // Create two triangles for each quad
            float vertices[6][3] = {
                {sphereVertices[p1 * 3], sphereVertices[p1 * 3 + 1], sphereVertices[p1 * 3 + 2]},
                {sphereVertices[p2 * 3], sphereVertices[p2 * 3 + 1], sphereVertices[p2 * 3 + 2]},
                {sphereVertices[p3 * 3], sphereVertices[p3 * 3 + 1], sphereVertices[p3 * 3 + 2]},
                {sphereVertices[p3 * 3], sphereVertices[p3 * 3 + 1], sphereVertices[p3 * 3 + 2]},
                {sphereVertices[p2 * 3], sphereVertices[p2 * 3 + 1], sphereVertices[p2 * 3 + 2]},
                {sphereVertices[p4 * 3], sphereVertices[p4 * 3 + 1], sphereVertices[p4 * 3 + 2]}
            };

            glBegin(GL_TRIANGLES);
            for (int i = 0; i < 6; ++i) {
                glColor3fv(&sphereColors[(p1 + i) * 3]);
                glVertex3fv(vertices[i]);
            }
            glEnd();
        }
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glPopMatrix();
}


// Function to spawn particles at a specific position

//save it for late for the rain system
// void spawnParticle(float x, float y, float z) {
//     if (particles.size() < maxParticles) {
//         Particle newParticle;
//         newParticle.position[0] = x;
//         newParticle.position[1] = y;
//         newParticle.position[2] = z;

//         // Randomize initial velocity for more dynamic behavior
//         newParticle.velocity[0] = (rand() % 100) / 100.0f - 0.5f;  // Random x velocity
//         newParticle.velocity[1] = 0.0f;  // Initial y velocity (can be set to any value)
//         newParticle.velocity[2] = (rand() % 100) / 100.0f - 0.5f;  // Random z velocity

//         newParticle.isActive = true;

//         particles.push_back(newParticle);
//     }
// }

// Random function helper with more granularity
float randomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

void spawnParticle(float centerX, float centerY, float centerZ) {
    if (particles.size() < maxParticles) {
        Particle newParticle;

        // Randomize starting positions within a range around the center
        newParticle.position[0] = randomFloat(centerX - 1.0f * M_PI, centerX + 1.0f);
        newParticle.position[1] = randomFloat(centerY - 1.0f * M_PI, centerY + 1.0f);
        newParticle.position[2] = randomFloat(centerZ - 1.0f * M_PI, centerZ + 1.0f);

        // Ensure velocities have enough randomness
        newParticle.velocity[0] = randomFloat(-0.5f, 0.5f) + pi * 0.05f;
        newParticle.velocity[1] = randomFloat(-0.5f, 0.5f) + pi * 0.05f;
        newParticle.velocity[2] = randomFloat(-0.5f, 0.5f) + pi * 0.05f;

        newParticle.isActive = true;

        particles.push_back(newParticle);
    }
}

//works best 8/10
// void spawnParticle(float centerX, float centerY, float centerZ) {
//     if (particles.size() < maxParticles) {
//         Particle newParticle;

//         // Start positions with small randomness around (centerX, centerY, centerZ)
//         newParticle.position[0] = centerX + ((rand() % 200 - 100) / 100.0f); // ±1 unit
//         newParticle.position[1] = centerY + ((rand() % 200 - 100) / 100.0f);
//         newParticle.position[2] = centerZ + ((rand() % 200 - 100) / 100.0f);

//         // Initial random velocity with added pi offset for randomness
//         newParticle.velocity[0] = ((rand() % 100 - 50) / 100.0f) + pi * 0.1f;
//         newParticle.velocity[1] = ((rand() % 100 - 50) / 100.0f) + pi * 0.1f;
//         newParticle.velocity[2] = ((rand() % 100 - 50) / 100.0f) + pi * 0.1f;

//         newParticle.isActive = true;

//         particles.push_back(newParticle);
//     }
// }

// Function to update particles (apply gravity and movement)

//save it for later for the rain system
// void updateParticles() {
//     for (int i = 0; i < particles.size(); ++i) {
//         if (particles[i].isActive) {
//             // Update velocity due to gravity
//             particles[i].velocity[1] += gravity * deltaT;  // gravity affects y-velocity

//             // Update position based on velocity
//             particles[i].position[0] += particles[i].velocity[0] * deltaT;
//             particles[i].position[1] += particles[i].velocity[1] * deltaT;
//             particles[i].position[2] += particles[i].velocity[2] * deltaT;

//             // Despawn if the particle falls below ground level (e.g., y < 0)
//             if (particles[i].position[1] < 0.0f) {
//                 particles[i].isActive = false; // Deactivate the particle
//             }
//         }
//     }
// }

void updateParticles() {
    for (int i = 0; i < particles.size(); ++i) {
        if (particles[i].isActive) {
            // Apply oscillating randomness to each axis for unpredictable bounciness
            particles[i].velocity[0] += sin(particles[i].position[0] * pi * 0.1f) * 0.5f;
            particles[i].velocity[1] += cos(particles[i].position[1] * pi * 0.1f) * 0.5f;
            particles[i].velocity[2] += sin(particles[i].position[2] * pi * 0.1f) * 0.5f;

            // Update position with current velocity
            particles[i].position[0] += particles[i].velocity[0] * deltaT * speedMultiplier;
            particles[i].position[1] += particles[i].velocity[1] * deltaT * speedMultiplier;
            particles[i].position[2] += particles[i].velocity[2] * deltaT * speedMultiplier;

            // Bouncing effect by reversing direction at boundaries
            if (particles[i].position[0] > 10.0f || particles[i].position[0] < -10.0f)
                particles[i].velocity[0] = -particles[i].velocity[0];
            if (particles[i].position[1] > 10.0f || particles[i].position[1] < -10.0f)
                particles[i].velocity[1] = -particles[i].velocity[1];
            if (particles[i].position[2] > 10.0f || particles[i].position[2] < -10.0f)
                particles[i].velocity[2] = -particles[i].velocity[2];
        }
    }
}


// Function to check and despawn particles if needed

//save it for later for the rain system
// void checkAndDespawnParticles() {
//     if (particles.size() > maxParticles) {
//         // Deactivate and remove the first particle in the list
//         particles[0].isActive = false;
//         particles.erase(particles.begin());
//     }
// }



// Function to render all active particles

//save it for later for the rain system
// void renderParticles() {
//     for (int i = 0; i < particles.size(); ++i) {
//         if (particles[i].isActive) {
//             // Render each particle as a small sphere (use your existing render function)
//             renderSphere3D(0.1f, 10, 0.0f, particles[i].position[0], particles[i].position[1], particles[i].position[2]);
//         }
//     }
// }

void renderParticles() {
    for (int i = 0; i < particles.size(); ++i) {
        if (particles[i].isActive) {
            // Render each particle as a small sphere (use your existing render function)
            renderSphere3D(1.0f, segments, 0.0f, particles[i].position[0], particles[i].position[1], particles[i].position[2]);
        }
    }
}

// Update function that will be called in the display loop
void update() {
    // Randomly spawn particles at a fixed position (adjust spawn area as needed)
    spawnParticle(0.0f, 0.0f, 0.0f); // Spawn at (0, 10, 0)

    // Update particles (apply gravity and update positions)
    updateParticles();

    // Check if we need to despawn any old particles
    //checkAndDespawnParticles(); // Optional: Despawn particles if we exceed the maximum

    // Render active particles
    renderParticles();
}




void  renderCubeWithGrid(float size, int gridDivisions = 10) {
    float halfSize = size / 2.0f;
    float step = size / gridDivisions;

    // Render solid cube
    glBegin(GL_QUADS);
    glColor3f(0.5f, 0.5f, 0.5f);

    // Front face
    glVertex3f(-halfSize, -halfSize, halfSize);
    glVertex3f(halfSize, -halfSize, halfSize);
    glVertex3f(halfSize, halfSize, halfSize);
    glVertex3f(-halfSize, halfSize, halfSize);

    // Back face
    glVertex3f(-halfSize, -halfSize, -halfSize);
    glVertex3f(-halfSize, halfSize, -halfSize);
    glVertex3f(halfSize, halfSize, -halfSize);
    glVertex3f(halfSize, -halfSize, -halfSize);

    // Left face
    glVertex3f(-halfSize, -halfSize, -halfSize);
    glVertex3f(-halfSize, -halfSize, halfSize);
    glVertex3f(-halfSize, halfSize, halfSize);
    glVertex3f(-halfSize, halfSize, -halfSize);

    // Right face
    glVertex3f(halfSize, -halfSize, -halfSize);
    glVertex3f(halfSize, halfSize, -halfSize);
    glVertex3f(halfSize, halfSize, halfSize);
    glVertex3f(halfSize, -halfSize, halfSize);

    // Top face
    glVertex3f(-halfSize, halfSize, -halfSize);
    glVertex3f(-halfSize, halfSize, halfSize);
    glVertex3f(halfSize, halfSize, halfSize);
    glVertex3f(halfSize, halfSize, -halfSize);

    // Bottom face
    glVertex3f(-halfSize, -halfSize, -halfSize);
    glVertex3f(halfSize, -halfSize, -halfSize);
    glVertex3f(halfSize, -halfSize, halfSize);
    glVertex3f(-halfSize, -halfSize, halfSize);

    glEnd();

    // Overlay grid lines on each face
    glColor3f(0.0f, 0.0f, 0.0f);  // Set grid color to black

    glBegin(GL_LINES);

    // Front face grid
    for (float i = -halfSize; i <= halfSize; i += step) {
        glVertex3f(i, -halfSize, halfSize); glVertex3f(i, halfSize, halfSize);  // Vertical lines
        glVertex3f(-halfSize, i, halfSize); glVertex3f(halfSize, i, halfSize);  // Horizontal lines
    }

    // Back face grid
    for (float i = -halfSize; i <= halfSize; i += step) {
        glVertex3f(i, -halfSize, -halfSize); glVertex3f(i, halfSize, -halfSize);
        glVertex3f(-halfSize, i, -halfSize); glVertex3f(halfSize, i, -halfSize);
    }

    // Left face grid
    for (float i = -halfSize; i <= halfSize; i += step) {
        glVertex3f(-halfSize, -halfSize, i); glVertex3f(-halfSize, halfSize, i);
        glVertex3f(-halfSize, i, -halfSize); glVertex3f(-halfSize, i, halfSize);
    }

    // Right face grid
    for (float i = -halfSize; i <= halfSize; i += step) {
        glVertex3f(halfSize, -halfSize, i); glVertex3f(halfSize, halfSize, i);
        glVertex3f(halfSize, i, -halfSize); glVertex3f(halfSize, i, halfSize);
    }

    // Top face grid
    for (float i = -halfSize; i <= halfSize; i += step) {
        glVertex3f(i, halfSize, -halfSize); glVertex3f(i, halfSize, halfSize);
        glVertex3f(-halfSize, halfSize, i); glVertex3f(halfSize, halfSize, i);
    }

    // Bottom face grid
    for (float i = -halfSize; i <= halfSize; i += step) {
        glVertex3f(i, -halfSize, -halfSize); glVertex3f(i, -halfSize, halfSize);
        glVertex3f(-halfSize, -halfSize, i); glVertex3f(halfSize, -halfSize, i);
    }

    glEnd();
}

void renderCubeMap() {
    // Set adjustable position coordinates within the function
    float x = 0.0f;  // Change this value to adjust the x position
    float y = 16.0f;   // Change this value to adjust the y position
    float z = 0.0f; // Change this value to adjust the z position

    float size = 36.0f;          // Size of each cube side
    int gridDivisions = 12;      // Number of divisions for the grid effect

    glPushMatrix();
    glTranslatef(x, y, z);       // Adjust the position of the cube using the set coordinates
    renderCubeWithGrid(size, gridDivisions);  // Render the cube with grid lines
    glPopMatrix();
}


void timer(int value) {
    updateMovement();
    glutPostRedisplay();
    glutTimerFunc(refreshMills, timer, 0);
}

void reshape(GLsizei width, GLsizei height) {
    if (height == 0) height = 1; // Prevent division by zero
    float aspect = (float)width / (float)height;

    glViewport(0, 0, width, height); // Set the viewport to the new window size
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    //             fov     aspect ratio  near plane  far plane
    gluPerspective(45.0f, aspect, 0.1f, 100.0f); // Set the perspective view
}

void renderFPS() {
    // Calculate FPS
    frameCount++;
    auto currentTime = chrono::high_resolution_clock::now();
    chrono::duration<float> elapsedTime = currentTime - startTime;
    if (elapsedTime.count() >= 1.0f) {
        fps = frameCount / elapsedTime.count();
        frameCount = 0;
        startTime = currentTime;
    }

    // Switch to orthographic projection to render FPS
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Render FPS
    stringstream ss;
    ss << "FPS: " << fps;
    string fpsString = ss.str();
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2i(10, glutGet(GLUT_WINDOW_HEIGHT) - 20); // Position the text at the top-left corner
    for (char c : fpsString) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    // Restore the previous projection matrix
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}



void renderText(float x, float y, const string& text) {
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }
}

void displayCoordinates(float coordX, float coordY, float coordZ) {
    // Convert the coordinates to a string for display
    ostringstream oss;
    oss << "Coordinates: X = " << coordX << ", Y = " << coordY << ", Z = " << coordZ;
    string coordText = oss.str();

    // Retrieve the viewport dimensions
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    int viewportWidth = viewport[2]; // Get the viewport width
    int viewportHeight = viewport[3]; // Get the viewport height

    // Set up orthographic projection to display text in 2D screen space
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, viewportWidth, 0, viewportHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Calculate position dynamically based on viewport dimensions
    float xPos = viewportWidth - 350.0f; // Padding from the right edge 350original if it increase it goes to the left most
    float yPos = viewportHeight - 20.0f; // Padding from the top edge 20original

    // Render the text at the calculated position
    glColor3f(1.0f, 1.0f, 1.0f);  // Set text color to white
    renderText(xPos, yPos, coordText);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}


void renderHUD() {
    if (!hudEnabled) return;  // Only render if HUD is enabled

    // Set up orthographic projection for 2D overlay
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    gluOrtho2D(0, viewport[2], 0, viewport[3]);  // Match viewport dimensions

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Position HUD in bottom-left corner
    glColor3f(1.0f, 1.0f, 1.0f);  // Set HUD text color to white
    int hudX = 10;
    int hudY = 100;

    renderText(hudX, hudY, "HUD - Controls:");
    renderText(hudX, hudY - 20, "W/A/S/D: Move");
    renderText(hudX, hudY - 40, "Q/E: Move Up/Down");
    renderText(hudX, hudY - 60, "R: Rotate");
    renderText(hudX, hudY - 80, "H: Toggle HUD");

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}







void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Set camera position and orientation
    gluLookAt(cameraX, cameraY, cameraZ, cameraX + lookX, cameraY + lookY, cameraZ + lookZ, 0.0f, 1.0f, 0.0f);
    // Render the complete cube map (floor, ceiling, and walls)
    renderCubeMap();

    // Render shapes
    renderCube();         // Only render cube once
    renderPyramid();
    //            size of sphere  number of segments  angle of rotation  x position  y position  z position
    renderSphere3D(0.01f, segments, angleCircle, -2.0f, 0.0f, 5.0f); //this is vbo
    renderAnotherSphere3D(1.0f, 100, angleCircle, 1.0f, 0.0f, 5.0f); //no vbo

    // Render pause menu overlay if game is paused
    renderPauseMenu();

    renderFPS();

    // Display the coordinates of the camera
    displayCoordinates(cameraX, cameraY, cameraZ);

    //displays the HUD
    renderHUD();

    // display particle system
    updateParticles();
    renderParticles();

    update();

    glutSwapBuffers();

    if (!isPaused) {
        anglePyramid += 0.2f;
        angleCube -= 0.15f;
        angleCircle += 0.2f;
    }
}