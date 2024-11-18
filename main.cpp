#define GLEW_STATIC
#define STB_IMAGE_IMPLEMENTATION

#include "stb/stb_image.h"

#include <GL/glew.h>  // Added for VBO support
#include <GL/glut.h>
#include <GL/freeglut_ext.h>
#include <math.h>
#include <string.h>
#include <chrono>
#include <sstream>
#include <vector>
#include <algorithm>
#include <random>

#include <fstream>
#include <ctime>
#include <direct.h> //for _getcwd
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "textureloader.cpp" //texture method
#include "interaction.h" //movement globals
#include "particlesys.h" //particle system global var
#include "pyramid.h" //pyramid coords
#include "skybox.h" //skybox float vertices

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
void renderPyramid(); //function for pyramid instances
void initPyramidInstances();
void renderSphere3D(float radius, int segments, float angle, float xPos, float yPos, float zPos);
void skyBoxMap();
void updateParticles();
void renderParticles();
void displayCoordinates(float coordX, float coordY, float coordZ);
void renderHUD();

void initVBOs(); // Initialize VBOs
void cleanupVBO(); // Cleanup VBOs
void cleanup();

GLuint loadTexture(const char* filename); // Load texture from file


/* Global variables */
//Initialize VBOs
GLuint pyramidVBOs[2], sphere3DVBOs[2], skyBoxVBOs[4];


// Texture IDs for different surfaces
struct SkyboxTextures {
    GLuint floor, roof, frontWall, backWall, leftWall, rightWall;
} skyboxTextures;

struct Textures { // Textures for different surfaces
    GLuint pyramid;
} textures;


// Add structure for pyramid instances
struct PyramidInstance {
    glm::vec3 position;
    float rotation;
    glm::vec3 scale;
};

//Draw fps
chrono::time_point<chrono::high_resolution_clock> startTime;
int frameCount = 0;
float fps = 0.0f;
//Hud display
bool hudEnabled = true;

//number of segments for the sphere (higher is laggier) but may bug out 20 is best and smooth circle
const int segments = 10;

// Add these global variables at the top for rendercubegrid
float lightAngle = 80.0f;
float lightRadius = 400.0f;
float lightY = 150.0f; //light height
float lightSpeed = 0.5f;
int currentCorner = 3;

//for logging purposes
bool resourcesInitialized = false;
ofstream logFile;
string logPath;

vector<float> vertexData; //vertex data for skybox
vector<Particle> particles; //particle system
vector<float> textureCoordData; //texture coordinate data
vector<PyramidInstance> pyramidInstances; //vector to store instances


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

    // Initialize GLEW for OpenGL extension support
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW initialization failed: %s\n", glewGetErrorString(err));
        return 1;
    }
    else {
        fprintf(stdout, "GLEW initialized successfully\n");
    }

    initGL();

    glutTimerFunc(0, timer, 0);
    glutMainLoop();
    return 0;
}

//logging or log files
void logMessage(const char* message) {
    if (!logFile.is_open()) {
        char cwd[256];
        _getcwd(cwd, sizeof(cwd));
        logPath = string(cwd) + "\\opengl_cleanup.log";
        logFile.open(logPath, ios::app);

        // Print the log file location when it's first created
        cout << "Log file created at: " << logPath << endl;
    }

    // Get current time
    time_t now = time(0);
    struct tm localTime;
    localtime_s(&localTime, &now); // Safer version of localtime

    // Format the time as 12-hour format with AM/PM
    char formattedTime[50];
    strftime(formattedTime, sizeof(formattedTime), "%I:%M:%S %p", &localTime);

    // Write to both log file and console
    cout << formattedTime << ": " << message << endl;
    logFile << formattedTime << ": " << message << endl;
    logFile.flush();
}

void initGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);

    initPyramidInstances();

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glutSetCursor(GLUT_CURSOR_NONE);
    initVBOs();

    startTime = chrono::high_resolution_clock::now();
    logMessage("Application starting Now...");

    glEnable(GL_TEXTURE_2D);

    // Load all textures
    skyboxTextures.floor = loadTexture("c:/Users/buanh/Documents/VSCODE_SAVES/OpenGL/Projects/3D_WORLD_BUAN/sand.png");
    skyboxTextures.backWall = loadTexture("c:/Users/buanh/Documents/VSCODE_SAVES/OpenGL/Projects/3D_WORLD_BUAN/skyclouds.png");
    skyboxTextures.frontWall = loadTexture("c:/Users/buanh/Documents/VSCODE_SAVES/OpenGL/Projects/3D_WORLD_BUAN/skyclouds.png");
    skyboxTextures.leftWall = loadTexture("c:/Users/buanh/Documents/VSCODE_SAVES/OpenGL/Projects/3D_WORLD_BUAN/skyclouds.png");
    skyboxTextures.rightWall = loadTexture("c:/Users/buanh/Documents/VSCODE_SAVES/OpenGL/Projects/3D_WORLD_BUAN/skyclouds.png");
    skyboxTextures.roof = loadTexture("c:/Users/buanh/Documents/VSCODE_SAVES/OpenGL/Projects/3D_WORLD_BUAN/sky.png");

    textures.pyramid = loadTexture("c:/Users/buanh/Documents/VSCODE_SAVES/OpenGL/Projects/3D_WORLD_BUAN/sandstone.jpg");
    atexit(cleanup);
}

// Initialize pyramid instances in initGL() or separate init function
void initPyramidInstances() {
    // Create 1000 pyramids with random positions across the skybox
    for (int i = 0; i < 30; i++) {
        PyramidInstance instance;
        instance.position = glm::vec3(
            randomFloat(-500.0f, 500.0f),     // x range within skybox
            randomFloat(10.0f, 37.5f),       // y range for varied heights
            randomFloat(-500.0f, 500.0f)      // z range within skybox
        );
        instance.scale = glm::vec3(randomFloat(1.0f, 10.0f)); // Random sizes
        pyramidInstances.push_back(instance);
    }
}

void renderPyramid() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures.pyramid);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBOs[0]);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBOs[1]);
    glTexCoordPointer(2, GL_FLOAT, 0, 0);

    // Render each instance
    for (const auto& instance : pyramidInstances) {
        glPushMatrix();
        glTranslatef(instance.position.x, instance.position.y, instance.position.z);
        glScalef(instance.scale.x, instance.scale.y, instance.scale.z);

        glDrawArrays(GL_TRIANGLES, 0, 12);
        glDrawArrays(GL_QUADS, 12, 4);

        glPopMatrix();
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
}
void pyramidVBO() {
    // Pyramid VBO setup
    glGenBuffers(2, pyramidVBOs);

    // Pyramid vertex VBO
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);
    // Pyramid color VBO
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidTextureCoords), pyramidTextureCoords, GL_STATIC_DRAW);
}

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

    // Restore the original state
    glPopAttrib();  // Restore the original state

    glPushMatrix(); // Save the modelview matrix
    glPopMatrix(); // Restore the modelview matrix
}
void sphere3dVBO() {
    //Sphere3D VBO setup
    glGenBuffers(2, sphere3DVBOs);
    // Generate sphere vertex data and color data before sending them to the VBO
    const int numVertices = (segments + 1) * (segments + 1);
    float sphereVertices[numVertices * 3];
    float sphereColors[numVertices * 3];
    int index = 0;

    for (int lat = 0; lat <= segments; ++lat) {
        float theta = lat * M_PI / segments; // Latitude angle
        for (int lon = 0; lon <= segments; ++lon) {
            float phi = lon * 2.0f * M_PI / segments; // Longitude angle

            // Calculate x, y, z positions for the sphere vertex
            sphereVertices[index * 3] = sin(theta) * cos(phi); // x
            sphereVertices[index * 3 + 1] = sin(theta) * sin(phi); // y
            sphereVertices[index * 3 + 2] = cos(theta); // z

            // Color pattern for beach ball
            if (lon % 3 == 0) {  // Alternate colors
                sphereColors[index * 3] = 1.0f;  // Red component
                sphereColors[index * 3 + 1] = 1.0f;  // Green component
                sphereColors[index * 3 + 2] = 1.0f;  // Blue component
            }
            else if (lon % 3 == 1) {
                sphereColors[index * 3] = 1.0f;  // Red component
                sphereColors[index * 3 + 1] = 1.0f;  // Green component (Yellow)
                sphereColors[index * 3 + 2] = 1.0f;  // Blue component
            }
            else {
                sphereColors[index * 3] = 1.0f;  // Red component
                sphereColors[index * 3 + 1] = 1.0f;  // Green component
                sphereColors[index * 3 + 2] = 1.0f;  // Blue component
            }
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

void skyBoxVBO() {
    // Generate VBO IDs
    glGenBuffers(3, skyBoxVBOs);

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, skyBoxVBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    // Upload texture coordinate data
    glBindBuffer(GL_ARRAY_BUFFER, skyBoxVBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, textureCoordData.size() * sizeof(float), textureCoordData.data(), GL_STATIC_DRAW);

    // Enable texture and vertex arrays
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    // Set up vertex and texture coordinate pointers
    glBindBuffer(GL_ARRAY_BUFFER, skyBoxVBOs[0]);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, skyBoxVBOs[1]);
    glTexCoordPointer(2, GL_FLOAT, 0, 0);

    // Draw front wall
    glBindTexture(GL_TEXTURE_2D, skyboxTextures.frontWall);
    glDrawArrays(GL_QUADS, 0, 4);

    // Draw back wall
    glBindTexture(GL_TEXTURE_2D, skyboxTextures.backWall);
    glDrawArrays(GL_QUADS, 4, 4);

    // Draw left wall
    glBindTexture(GL_TEXTURE_2D, skyboxTextures.leftWall);
    glDrawArrays(GL_QUADS, 8, 4);

    // Draw right wall
    glBindTexture(GL_TEXTURE_2D, skyboxTextures.rightWall);
    glDrawArrays(GL_QUADS, 12, 4);

    // Draw roof
    glBindTexture(GL_TEXTURE_2D, skyboxTextures.roof);
    glDrawArrays(GL_QUADS, 16, 4);

    // Draw floor
    glBindTexture(GL_TEXTURE_2D, skyboxTextures.floor);
    // glDrawArrays(GL_QUADS, 20, 4);
    glDrawArrays(GL_TRIANGLE_STRIP, 20, Skybox::subDividedFloorVertices.size() / 3);

    // Cleanup
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
}
void skyBox(float size, int gridDivisions = 4) {
    float step = size / gridDivisions;
    vertexData.clear();
    textureCoordData.clear();

    Skybox::updateCubeVertices(size);
    Skybox::subdivideFloor(gridDivisions, size);
    // Apply heightmap with custom amplitude and frequency
    Skybox::applyHeightmap(0.0f, 1.01f); // Adjust these values for different terrain effects

    // Front Wall
    for (size_t i = 0; i < 12; i += 3) {
        vertexData.push_back(Skybox::frontWallVertices[i]);
        vertexData.push_back(Skybox::frontWallVertices[i + 1]);
        vertexData.push_back(Skybox::frontWallVertices[i + 2]);

        float texU = (Skybox::frontWallVertices[i] + size / 2) / size;
        float texV = Skybox::frontWallVertices[i + 1] / size;
        textureCoordData.push_back(texU);
        textureCoordData.push_back(texV);
    }

    // Back Wall
    for (size_t i = 0; i < 12; i += 3) {
        vertexData.push_back(Skybox::backWallVertices[i]);
        vertexData.push_back(Skybox::backWallVertices[i + 1]);
        vertexData.push_back(Skybox::backWallVertices[i + 2]);

        float texU = (Skybox::backWallVertices[i] + size / 2) / size;
        float texV = Skybox::backWallVertices[i + 1] / size;
        textureCoordData.push_back(texU);
        textureCoordData.push_back(texV);
    }

    // Left Wall
    for (size_t i = 0; i < 12; i += 3) {
        vertexData.push_back(Skybox::leftWallVertices[i]);
        vertexData.push_back(Skybox::leftWallVertices[i + 1]);
        vertexData.push_back(Skybox::leftWallVertices[i + 2]);

        float texU = (Skybox::leftWallVertices[i + 2] + size / 2) / size;
        float texV = Skybox::leftWallVertices[i + 1] / size;
        textureCoordData.push_back(texU);
        textureCoordData.push_back(texV);
    }

    // Right Wall
    for (size_t i = 0; i < 12; i += 3) {
        vertexData.push_back(Skybox::rightWallVertices[i]);
        vertexData.push_back(Skybox::rightWallVertices[i + 1]);
        vertexData.push_back(Skybox::rightWallVertices[i + 2]);

        float texU = (Skybox::rightWallVertices[i + 2] + size / 2) / size;
        float texV = Skybox::rightWallVertices[i + 1] / size;
        textureCoordData.push_back(texU);
        textureCoordData.push_back(texV);
    }

    // Roof
    for (size_t i = 0; i < 12; i += 3) {
        vertexData.push_back(Skybox::roofVertices[i]);
        vertexData.push_back(Skybox::roofVertices[i + 1]);
        vertexData.push_back(Skybox::roofVertices[i + 2]);

        float texU = (Skybox::roofVertices[i] + size / 2) / size;
        float texV = (Skybox::roofVertices[i + 2] + size / 2) / size;
        textureCoordData.push_back(texU);
        textureCoordData.push_back(texV);
    }

    // Floor with texture coordinates and grid lines
    // First add the main floor vertices
    for (size_t i = 0; i < 12; i += 3) {
        float x = Skybox::floorVertices[i];
        float y = Skybox::floorVertices[i + 1];
        float z = Skybox::floorVertices[i + 2];

        vertexData.push_back(x);
        vertexData.push_back(y);
        vertexData.push_back(z);

        float texU = (x + size / 2) / size;
        float texV = (z + size / 2) / size;
        textureCoordData.push_back(texU);
        textureCoordData.push_back(texV);
    }
}
void skyBoxMap() {
    float position[3] = { 0.0f, 16.0f, 0.0f };
    float size = 1024.0f;
    int gridDivisions = 36;
    float halfSize = size / 2.0f;

    // Define corner positions
    float corners[4][3] = {
        {-halfSize, lightY + halfSize, -halfSize},
        {halfSize, lightY + halfSize, -halfSize},
        {halfSize, lightY + halfSize, halfSize},
        {-halfSize, lightY + halfSize, halfSize}
    };

    // Calculate interpolated position between corners
    int nextCorner = (currentCorner + 1) % 4;
    float t = fmod(lightAngle, 90.0f) / 90.0f;

    // Interpolate position
    float lightPos[3];
    for (int i = 0; i < 3; i++) {
        lightPos[i] = corners[currentCorner][i] * (1.0f - t) +
            corners[nextCorner][i] * t;
        // Scale by light radius
        lightPos[i] *= (lightRadius / halfSize);
    }

    GLfloat lightPosition[] = { lightPos[0], lightPos[1], lightPos[2], 1.0f };

    // glEnable(GL_LIGHTING);
    // glEnable(GL_LIGHT0);

    GLfloat lightAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat lightDiffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    GLfloat materialAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat materialDiffuse[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat materialSpecular[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat materialShininess[] = { 100.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, materialShininess);

    skyBox(size, gridDivisions);

    glPushMatrix();
    {
        glTranslatef(position[0], position[1], position[2]);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f);

        // Only call skyBoxVBO here
        skyBoxVBO();

        glDisable(GL_POLYGON_OFFSET_FILL);
    }
    glPopMatrix();

    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);

    if (!isPaused) {
        lightAngle += lightSpeed;
        if (lightAngle >= 360.0f) {
            lightAngle = 0.0f;
            currentCorner = 0;
        }
        else if (fmod(lightAngle, 90.0f) < lightSpeed) {
            currentCorner = (currentCorner + 1) % 4;
        }
    }
}

void initVBOs() {
    pyramidVBO();
    sphere3dVBO();
    //skyBoxVBO();

    // Unbind buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    resourcesInitialized = true;
    logMessage("OpenGL resources initialized successfully");
}

void cleanupVBO() {
    if (resourcesInitialized) {
        if (glutGetWindow()) {  // Ensure a valid OpenGL context
            // Cleanup pyramid VBOs
            if (pyramidVBOs[0] || pyramidVBOs[1]) {
                glDeleteBuffers(2, pyramidVBOs);
                logMessage("Pyramid VBO resources successfully deleted.\n");
            }
            else {
                logMessage("Pyramid VBO resources were not initialized or already deleted.\n");
            }
            if (textures.pyramid) {
                glDeleteTextures(1, &textures.pyramid);
                logMessage("Pyramid texture successfully deleted.\n");
            }
            else {
                logMessage("Pyramid texture was not initialized or already deleted.\n");
            }
            // Cleanup sphere VBOs
            if (sphere3DVBOs[0] || sphere3DVBOs[1]) {
                glDeleteBuffers(2, sphere3DVBOs);
                logMessage("Sphere VBO resources successfully deleted.\n");
            }
            else {
                logMessage("Sphere VBO resources were not initialized or already deleted.\n");
            }
            if (skyboxTextures.floor || skyboxTextures.frontWall || skyboxTextures.backWall || skyboxTextures.leftWall || skyboxTextures.rightWall || skyboxTextures.roof) {
                GLuint textures[] = { skyboxTextures.floor, skyboxTextures.frontWall, skyboxTextures.backWall, skyboxTextures.leftWall, skyboxTextures.rightWall, skyboxTextures.roof };
                glDeleteTextures(6, textures);
                logMessage("Texture resources successfully deleted.\n");
            }
            else {
                logMessage("Texture resources were not initialized or already deleted.\n");
            }
            if (skyBoxVBOs[0] || skyBoxVBOs[1]) {
                glDeleteBuffers(2, skyBoxVBOs);
                logMessage("Render Cube with Grid VBO resources successfully deleted.\n");
            }
            else {
                logMessage("Render Cube with Grid VBO resources were not initialized or already deleted.\n");
            }
            resourcesInitialized = false;  // Mark resources as cleaned up
        }
        else {
            logMessage("Warning: No valid OpenGL context during cleanup.\n");
        }
    }
    else {
        logMessage("Resources are not initialized or have already been cleaned up.\n");
    }
}
void cleanup() {
    cleanupVBO();
    if (glutGetWindow()) {
        glutDestroyWindow(glutGetWindow());
        logMessage("OpenGL context cleaned up.");
    }
    if (logFile.is_open()) {
        logMessage("Closing log file.");
        logFile.close();
    }
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
void updateCameraDirection() {
    lookX = cos(yaw * M_PI / 180.0f) * cos(pitch * M_PI / 180.0f);
    lookY = sin(pitch * M_PI / 180.0f);
    lookZ = sin(yaw * M_PI / 180.0f) * cos(pitch * M_PI / 180.0f);
}
void keyboardDown(unsigned char key, int x, int y) {
    keyStates[key] = true;

    if (key == 27) { // ESC key
        if (!isPaused) {
            isPaused = true;
            glutSetCursor(GLUT_CURSOR_INHERIT);
            logMessage("Game Paused.");
        }
        else {
            isPaused = false;
            glutSetCursor(GLUT_CURSOR_NONE);
            logMessage("Game Resumed.");
        }
    }

    // Exit only if 'q' or 'Q' is pressed and the system is paused
    if (isPaused && (key == 'q' || key == 'Q')) {
        logMessage("Application Exited by user.\n");
        cleanup();
        exit(0);
    }

    if (key == 'h' || key == 'H') {
        hudEnabled = !hudEnabled; // Toggles HUD visibility
        if (hudEnabled) {
            logMessage("HUD toggled on.");
        }
        else {
            logMessage("HUD toggled off.");
        }
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

//particle system
//Set spawn area dimensions original
void setSpawnArea(
    float centerX,
    float centerY,
    float centerZ,
    float rangeX = 20.0f,
    float rangeY = 20.0f,
    float rangeZ = 20.0f) {

    spawnArea.centerX = centerX;
    spawnArea.centerY = centerY;
    spawnArea.centerZ = centerZ;
    spawnArea.rangeX = rangeX;
    spawnArea.rangeY = rangeY;
    spawnArea.rangeZ = rangeZ;
}

void spawnParticle(float centerX = -1.0f, float centerY = -1.0f, float centerZ = -1.0f) {
    if (particles.size() < maxParticles) {
        Particle newParticle;

        // Use provided coordinates or default spawn area
        float spawnX = (centerX >= 0) ? centerX : spawnArea.centerX;
        float spawnY = (centerY >= 0) ? centerY : spawnArea.centerY;
        float spawnZ = (centerZ >= 0) ? centerZ : spawnArea.centerZ;

        // Generate position within spawn area
        newParticle.position[0] = randomFloat(spawnX - spawnArea.rangeX, spawnX + spawnArea.rangeX);
        newParticle.position[1] = randomFloat(spawnY - spawnArea.rangeY, spawnY + spawnArea.rangeY);
        newParticle.position[2] = randomFloat(spawnZ - spawnArea.rangeZ, spawnZ + spawnArea.rangeZ);

        // Calculate depth factor based on spawn position
        newParticle.depthFactor = (newParticle.position[2] - (spawnZ - spawnArea.rangeZ)) / (2 * spawnArea.rangeZ);

        // Set velocities (primarily moving left)
        float baseSpeed = calculateSpeedForDepth(newParticle.depthFactor);
        newParticle.velocity[0] = -baseSpeed + randomFloat(-2.0f, 2.0f);
        newParticle.velocity[1] = randomFloat(-2.0f, 2.0f) * newParticle.depthFactor;
        newParticle.velocity[2] = randomFloat(-1.0f, 1.0f) * (1.0f - newParticle.depthFactor);

        // Set visual properties
        newParticle.size = calculateSizeForDepth(newParticle.depthFactor);
        newParticle.isActive = true;

        particles.push_back(newParticle);
    }
}
void initializeParticles() {
    // Initially populate the system with particles across the entire width
    for (int i = 0; i < maxParticles; ++i) {
        if (particles.size() < maxParticles) {
            Particle newParticle;
            newParticle.position[0] = randomFloat(leftBoundary, rightBoundary);
            newParticle.position[1] = randomFloat(-verticalRange, verticalRange);
            newParticle.position[2] = randomFloat(-depthRange, depthRange);

            newParticle.velocity[0] = randomFloat(-15.0f, -10.0f);
            newParticle.velocity[1] = randomFloat(-25.0f, -20.0f);
            newParticle.velocity[2] = randomFloat(-35.0f, -30.0f);

            newParticle.size = randomFloat(0.5f, 2.0f);
            newParticle.isActive = true;

            particles.push_back(newParticle);
        }
    }
}
void updateParticles() {
    float respawnX = spawnArea.centerX;
    float respawnRange = spawnArea.rangeX;

    for (int i = 0; i < particles.size(); ++i) {
        if (particles[i].isActive) {
            // Update position
            particles[i].position[0] += particles[i].velocity[0] * deltaT * speedMultiplier;
            particles[i].position[1] += particles[i].velocity[1] * deltaT * speedMultiplier;
            particles[i].position[2] += particles[i].velocity[2] * deltaT * speedMultiplier;

            // Check if particle needs respawning
            if (particles[i].position[0] < spawnArea.centerX - spawnArea.rangeX * 3) {
                // Respawn at original spawn area
                particles[i].position[0] = randomFloat(respawnX - respawnRange, respawnX + respawnRange);
                particles[i].position[1] = randomFloat(spawnArea.centerY - spawnArea.rangeY,
                    spawnArea.centerY + spawnArea.rangeY);
                particles[i].position[2] = randomFloat(spawnArea.centerZ - spawnArea.rangeZ,
                    spawnArea.centerZ + spawnArea.rangeZ);

                // Recalculate depth factor
                particles[i].depthFactor = (particles[i].position[2] - (spawnArea.centerZ - spawnArea.rangeZ))
                    / (2 * spawnArea.rangeZ);

                // Reset velocities and properties
                float baseSpeed = calculateSpeedForDepth(particles[i].depthFactor);
                particles[i].velocity[0] = -baseSpeed + randomFloat(-2.0f, 2.0f);
                particles[i].velocity[1] = randomFloat(-2.0f, 2.0f) * particles[i].depthFactor;
                particles[i].velocity[2] = randomFloat(-1.0f, 1.0f) * (1.0f - particles[i].depthFactor);

                particles[i].size = calculateSizeForDepth(particles[i].depthFactor);
            }

            // Keep particles within vertical and depth bounds
            float verticalBound = spawnArea.rangeY * 2;
            float depthBound = spawnArea.rangeZ * 2;

            if (abs(particles[i].position[1] - spawnArea.centerY) > verticalBound) {
                particles[i].velocity[1] *= -0.5f;
            }
            if (abs(particles[i].position[2] - spawnArea.centerZ) > depthBound) {
                particles[i].velocity[2] *= -0.5f;
            }
        }
    }
}
void renderParticles() {
    // Sort particles by depth for proper rendering
    sort(particles.begin(), particles.end(),
        [](const Particle& a, const Particle& b) {
            return a.position[2] > b.position[2];
        });

    for (int i = 0; i < particles.size(); ++i) {
        if (particles[i].isActive) {
            renderSphere3D(particles[i].size, segments, 1.0f, particles[i].position[0], particles[i].position[1], particles[i].position[2]);
        }
    }
}
void update() {
    // Spawn new particles if needed
    if (particles.size() < maxParticles) {
        spawnParticle();
    }
    updateParticles();
    renderParticles();
}


void reshape(GLsizei width, GLsizei height) { //perspective projection
    // Prevent division by zero
    if (height == 0) height = 1;
    float aspect = (float)width / (float)height;

    // Set the viewport to the new window size
    glViewport(0, 0, width, height);
    // Switch to projection matrix mode
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Adjusted perspective parameters for better cube visualization
    // Increased FOV to 65 for better cube viewing
    // Far plane increased to 10000 units (it renders this nth units)(frustrum culling) to see cubes from greater distances 
    gluPerspective(45.0f, aspect, 0.5f, 10000.0f);

    // Enable necessary rendering features
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL); // Less than or equal depth comparison for better depth precision

    // //Enable back face culling for better cube rendering (frustrum culling)
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);
    // glCullFace(GL_FRONT);

    // Enable line smoothing for better grid lines
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    // Enable blending for smoother lines
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Setup fog for distance fade
    GLfloat fogColor[4] = { 0.96f, 0.64f, 0.38f, 1.0f }; // Desert sand color fog
    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_START, 10000.0f);     // Start fog after 2000 units 35 best nearest
    glFogf(GL_FOG_END, 50000.0f);       // Full fog by 4500 units 50 or100 for farthest
    glHint(GL_FOG_HINT, GL_NICEST);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
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
void timer(int value) {
    updateMovement();
    glutPostRedisplay();
    glutTimerFunc(refreshMills, timer, 0);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(cameraX, cameraY, cameraZ, cameraX + lookX, cameraY + lookY, cameraZ + lookZ, 0.0f, 1.0f, 0.0f);  // Set camera position and orientation
    skyBoxMap(); //skybox  // Render the complete cube map (floor, ceiling, and walls)

    // Render shapes
    renderPyramid();   //render pyramid
    renderSphere3D(1.0f, segments, angleCircle, -2.0f, 17.5f, 5.0f); //this is vbo  //size of sphere  number of segments  angle of rotation  x position  y position  z position

    renderPauseMenu(); // Render pause menu overlay if game is paused
    renderFPS(); //fps
    displayCoordinates(cameraX, cameraY, cameraZ); // Display the coordinates of the camera
    renderHUD(); //displays the HUD

    updateParticles();  // display particle system
    renderParticles(); // display particle system
    update(); //keeps updating to spawn particles

    glutSwapBuffers();

    if (!isPaused) {
        anglePyramid += 0.2f;
        angleCube -= 0.15f;
        angleCircle += 0.2f;
    }
}