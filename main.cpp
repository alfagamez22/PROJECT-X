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
#include <direct.h> //for _getcwd for manipulating file
#include <iostream>

using namespace std;

// Function prototypes
void initGL();
void display();
void timer(int value);
void reshape(GLsizei width, GLsizei height);
void keyboardDown(unsigned char key, int x, int y);
void keyboardUp(unsigned char key, int x, int y);
void mouseMovement(int x, int y);
void mouseWheel(int button, int dir, int x, int y);
void updateCameraDirection();
void updateMovement();
void renderPauseMenu();
void renderPyramid(); //function for pyramid instances
void initPyramidInstances();
void renderSun(float radius, int segments, float angle, float xPos, float yPos, float zPos);
void renderSkyBox();
void displayCoordinates(float coordX, float coordY, float coordZ);
void renderHUD();
void renderSunCore(int segments);
void simulateEnvironmentalLightEffects(float lightX, float lightY, float lightZ, float lightRadius);

void initVBOs(); // Initialize VBOs
void cleanupVBO(); // Cleanup VBOs
void cleanup();


/* Global variables */
//Initialize VBOs
GLuint pyramidVBOs[3], sunVBOs[3], skyBoxVBOs[2];

//Texture Loader
GLuint loadTexture(const char* filename) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        printf("Failed to load texture: %s\n", filename);
    }

    stbi_image_free(data);
    return textureID;
}

struct SkyboxTextures { // Texture IDs for different surfaces
    GLuint floor, roof, frontWall, backWall, leftWall, rightWall;
} skyboxTextures;
struct Textures { // Textures for different surfaces
    GLuint pyramid;
} textures;
struct PyramidInstance { //Instantiate the number of pyramids with randomness in x y z coordinates
    struct Vec3 {
        float x, y, z;
    };
    Vec3 position;
    float rotation;
    Vec3 scale;
};
float randomFloat(float min, float max) { //helper function for randomizing pyramid instances
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}
float anglePyramid = 0.0f;
float angleCube = 0.0f;
float angleCircle = 0.0f;  // Rotation angle for the circle

int refreshMills = 5; // Refresh interval in milliseconds the lower the better maximum 5 minimum 25 (15 best) to avoid visual bugs

//Camera Dimensions
// Camera position and orientation starting
float cameraX = 0.0f;
float cameraY = 17.5f;
float cameraZ = 10.0f;
float cameraSpeed = 2.1f; //can be adjusted the higher the faster 0.1 is the best it was 0.3 kinda quick
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

// Movement flags and velocity
bool keyStates[256] = { false };
float velocityX = 0.0f;
float velocityZ = 0.0f;
float maxVelocity = 1.15f;
float acceleration = 0.05f; //inertia //0.05 is the best
float deceleration = 0.1f; //inertia //0.01 is the best

// Window state
bool isPaused = false;
bool firstMouse = true;

//Sun Variables
float glowIntensity = 1.0f;
float glowSpeed = 0.01f;
float glowMin = 0.1f;
float glowMax = 1.0f;
bool glowIncreasing = true;

//fog
bool fogEnabled = true;

//Draw fps
chrono::time_point<chrono::high_resolution_clock> startTime;
int frameCount = 0;
float fps = 0.0f;
//Hud display
bool hudEnabled = true;

//number of segments for the sphere (higher is laggier) but may bug out 20 is best and smooth circle
const int segments = 100;

//for logging purposes
bool resourcesInitialized = false;
ofstream logFile;
string logPath;

//PYRAMID COORDINATES
// Pyramid vertices and colors
float scaling = 3.0f;
// Vertex and color arrays for the pyramid
float pyramidVertices[] = {
    // Front face triangle
    0.0f, scaling * 1.0f, 0.0f,  -scaling * 1.0f, -scaling * 1.0f, scaling * 1.0f,  scaling * 1.0f, -scaling * 1.0f, scaling * 1.0f,
    // Right face triangle
    0.0f, scaling * 1.0f, 0.0f,  scaling * 1.0f, -scaling * 1.0f, scaling * 1.0f,  scaling * 1.0f, -scaling * 1.0f, -scaling * 1.0f,
    // Back face triangle
    0.0f, scaling * 1.0f, 0.0f,  scaling * 1.0f, -scaling * 1.0f, -scaling * 1.0f,  -scaling * 1.0f, -scaling * 1.0f, -scaling * 1.0f,
    // Left face triangle
    0.0f, scaling * 1.0f, 0.0f,  -scaling * 1.0f, -scaling * 1.0f, -scaling * 1.0f,  -scaling * 1.0f, -scaling * 1.0f, scaling * 1.0f,
    // Bottom face square
    -scaling * 1.0f, -scaling * 1.0f,  scaling * 1.0f,
     scaling * 1.0f, -scaling * 1.0f,  scaling * 1.0f,
     scaling * 1.0f, -scaling * 1.0f, -scaling * 1.0f,
    -scaling * 1.0f, -scaling * 1.0f, -scaling * 1.0f
};
float pyramidTextureCoords[] = {
    // Front face triangle
    0.5f, 1.0f,    0.0f, 0.0f,    1.0f, 0.0f,
    // Right face triangle
    0.5f, 1.0f,    0.0f, 0.0f,    1.0f, 0.0f,
    // Back face triangle
    0.5f, 1.0f,    0.0f, 0.0f,    1.0f, 0.0f,
    // Left face triangle
    0.5f, 1.0f,    0.0f, 0.0f,    1.0f, 0.0f,
    // Bottom face square
    0.0f, 0.0f,    1.0f, 0.0f,    1.0f, 1.0f,    0.0f, 1.0f
};
//Normal vectors for the pyramid faces on how light reflects each of the face
static const GLfloat pyramidNormals[] = {
    // Front face
    0.0f, 0.5f, 1.0f,
    0.0f, 0.5f, 1.0f,
    0.0f, 0.5f, 1.0f,
    // Right face
    1.0f, 0.5f, 0.0f,
    1.0f, 0.5f, 0.0f,
    1.0f, 0.5f, 0.0f,
    // Back face
    0.0f, 0.5f, -1.0f,
    0.0f, 0.5f, -1.0f,
    0.0f, 0.5f, -1.0f,
    // Left face
    -1.0f, 0.5f, 0.0f,
    -1.0f, 0.5f, 0.0f,
    -1.0f, 0.5f, 0.0f,
    // Base (bottom face)
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, -1.0f, 0.0f
};

vector<float> vertexData; //vertex data for skybox
vector<float> textureCoordData; //texture coordinate data
vector<PyramidInstance> pyramidInstances; //vector to store instances

class Skybox {
public:
    static vector<float> frontWallVertices;
    static vector<float> backWallVertices;
    static vector<float> leftWallVertices;
    static vector<float> rightWallVertices;
    static vector<float> roofVertices;
    static vector<float> floorVertices;

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
    static void updateVertexArray(vector<float>& target, float* source, size_t size) {
        target.clear();
        for (size_t i = 0; i < size / sizeof(float); i++) {
            target.push_back(source[i]);
        }
    }
};

// Define the static member
vector<float> Skybox::frontWallVertices;
vector<float> Skybox::backWallVertices;
vector<float> Skybox::leftWallVertices;
vector<float> Skybox::rightWallVertices;
vector<float> Skybox::roofVertices;
vector<float> Skybox::floorVertices;


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
    glutMouseWheelFunc(mouseWheel);
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

void initPyramidInstances() {
    for (int i = 0; i < 1000; i++) {
        PyramidInstance instance;

        // Set position
        instance.position.x = randomFloat(-5000.0f, 5000.0f);
        instance.position.y = randomFloat(10.0f, 37.5f);
        instance.position.z = randomFloat(-5000.0f, 5000.0f);

        // Set scale uniformly
        float randomScale = randomFloat(1.0f, 10.0f);
        instance.scale.x = randomScale;
        instance.scale.y = randomScale;
        instance.scale.z = randomScale;

        pyramidInstances.push_back(instance);
    }
}
void renderPyramid() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textures.pyramid);

    // Enable lighting for the pyramid
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT2);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);  // Enable normal array for lighting

    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBOs[0]);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBOs[1]);
    glTexCoordPointer(2, GL_FLOAT, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBOs[2]);
    glNormalPointer(GL_FLOAT, 0, 0);

    // Render each instance
    // for (const auto& instance : pyramidInstances) {
    //     glPushMatrix();
    //     glTranslatef(instance.position.x, instance.position.y, instance.position.z);
    //     glScalef(instance.scale.x, instance.scale.y, instance.scale.z);

    //     glDrawArrays(GL_TRIANGLES, 0, 12);
    //     glDrawArrays(GL_QUADS, 12, 4);

    //     glPopMatrix();
    // }
    for (const auto& instance : pyramidInstances) {
        glPushMatrix();
        glTranslatef(instance.position.x, instance.position.y, instance.position.z);
        glScalef(instance.scale.x * scaling,
            instance.scale.y * scaling,
            instance.scale.z * scaling);

        glDrawArrays(GL_TRIANGLES, 0, 12);
        glDrawArrays(GL_QUADS, 12, 4);
        glPopMatrix();
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT2);
}
void pyramidVBO() {
    // Generate 3 VBOs for vertices, textures, and normals
    glGenBuffers(3, pyramidVBOs);

    // Pyramid vertex VBO
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), nullptr, GL_STATIC_DRAW);
    GLfloat* vertexPtr = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    if (vertexPtr) {
        memcpy(vertexPtr, pyramidVertices, sizeof(pyramidVertices));
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }

    // Pyramid texture VBO
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidTextureCoords), nullptr, GL_STATIC_DRAW);
    GLfloat* texPtr = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    if (texPtr) {
        memcpy(texPtr, pyramidTextureCoords, sizeof(pyramidTextureCoords));
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }

    // Pyramid normals VBO
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBOs[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidNormals), nullptr, GL_STATIC_DRAW);
    GLfloat* normalPtr = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    if (normalPtr) {
        memcpy(normalPtr, pyramidNormals, sizeof(pyramidNormals));
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
}


/**
 * Renders a sun object with a pulsating glow effect.
 *
 * This function sets up the lighting and material properties for the sun, and then renders the sun geometry using the pre-generated vertex, color, and normal data stored in vertex buffer objects (VBOs).
 * The sun's glow intensity is updated to create a pulsating effect, and the sun's position, scale, and orientation can be controlled through the function parameters.
 *
 * The radius of the sun.
 * The number of segments used to approximate the spherical sun geometry.
 * The angle of rotation for the sun.
 * The x-coordinate of the sun's position.
 * The y-coordinate of the sun's position.
 * The z-coordinate of the sun's position.
 */
void renderSun(float radius, int segments, float angle, float xPos, float yPos, float zPos) {
    // Update glow intensity
    glShadeModel(GL_SMOOTH);  // Enable smooth shading
    glEnable(GL_NORMALIZE);   // Ensure normals are normalized
    if (glowIncreasing) {
        glowIntensity += glowSpeed * 0.016f;
        if (glowIntensity >= glowMax) {
            glowIncreasing = false;
        }
    }
    else {
        glowIntensity -= glowSpeed * 0.016f;
        if (glowIntensity <= glowMin) {
            glowIncreasing = true;
        }
    }
    // Set up enhanced sun lighting with pulsating effect
    GLfloat sunLight[] = { xPos, yPos, zPos, 1.0f };
    GLfloat sunAmbient[] = { 0.9f * glowIntensity, 0.8f * glowIntensity, 0.7f * glowIntensity, 1.0f };
    GLfloat sunDiffuse[] = { 1.0f * glowIntensity, 0.95f * glowIntensity, 0.8f * glowIntensity, 1.0f };
    GLfloat sunSpecular[] = { 1.0f * glowIntensity, 1.0f * glowIntensity, 0.9f * glowIntensity, 1.0f };
    GLfloat sunEmission[] = { 0.9f * glowIntensity, 0.85f * glowIntensity, 0.7f * glowIntensity, 1.0f };

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_POSITION, sunLight);
    glLightfv(GL_LIGHT1, GL_AMBIENT, sunAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, sunDiffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, sunSpecular);

    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.001f);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.0001f);

    glMaterialfv(GL_FRONT, GL_EMISSION, sunEmission);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, sunDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, sunSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, 100.0f);

    glPushMatrix();
    glTranslatef(xPos, yPos, zPos);
    glScalef(radius, radius, radius);
    glRotatef(angle, 1.0f, 0.0f, 0.0f);
    glRotatef(angle, 0.0f, 1.0f, 0.0f);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER, sunVBOs[0]);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, sunVBOs[1]);
    glColorPointer(3, GL_FLOAT, 0, 0);

    renderSunCore(segments);
    // Draw outer glow layer with pulsating alpha
    glPushMatrix();
    glScalef(1.2f, 1.2f, 1.2f);
    glColor4f(1.0f * glowIntensity, 0.8f * glowIntensity, 0.0f, 0.3f * glowIntensity);
    renderSunCore(segments);
    glPopMatrix();

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glPopMatrix();

    simulateEnvironmentalLightEffects(xPos, yPos, zPos, radius);
    glShadeModel(GL_FLAT);
    glDisable(GL_BLEND);
    glDisable(GL_LIGHT1);
}
/**
 * Generates vertex, color, and normal data for a spherical sun object and uploads it to OpenGL vertex buffer objects (VBOs).
 * The sun is represented by a sphere with a specified number of segments.
 * The vertex positions, colors, and normals are calculated and stored in separate VBOs for efficient rendering.
 */
void sunVBO() {
    glGenBuffers(3, sunVBOs);
    const int numVertices = (segments + 1) * (segments + 1);
    float sphereVertices[numVertices * 3];
    float sphereColors[numVertices * 3];
    float sphereNormals[numVertices * 3];
    int index = 0;
    // Generate sphere data
    for (int lat = 0; lat <= segments; ++lat) {
        float theta = lat * M_PI / segments;
        for (int lon = 0; lon <= segments; ++lon) {
            float phi = lon * 2.0f * M_PI / segments;

            // Calculate vertex positions, normals, and colors
            sphereVertices[index * 3] = sin(theta) * cos(phi);
            sphereVertices[index * 3 + 1] = sin(theta) * sin(phi);
            sphereVertices[index * 3 + 2] = cos(theta);

            sphereNormals[index * 3] = sphereVertices[index * 3];
            sphereNormals[index * 3 + 1] = sphereVertices[index * 3 + 1];
            sphereNormals[index * 3 + 2] = sphereVertices[index * 3 + 2];

            float intensity = 0.5f + 0.5f * sin(index * 0.2f);
            sphereColors[index * 3] = 1.0f * intensity;
            sphereColors[index * 3 + 1] = 0.8f * intensity;
            sphereColors[index * 3 + 2] = 0.2f * intensity;

            ++index;
        }
    }
    //upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, sunVBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * numVertices * 3, nullptr, GL_STATIC_DRAW);
    GLfloat* vertexPtr = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    if (vertexPtr) {
        memcpy(vertexPtr, sphereVertices, sizeof(float) * numVertices * 3);
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
    //upload color data
    glBindBuffer(GL_ARRAY_BUFFER, sunVBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * numVertices * 3, nullptr, GL_STATIC_DRAW);
    GLfloat* colorPtr = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    if (colorPtr) {
        memcpy(colorPtr, sphereColors, sizeof(float) * numVertices * 3);
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
    //upload normal data
    glBindBuffer(GL_ARRAY_BUFFER, sunVBOs[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * numVertices * 3, nullptr, GL_STATIC_DRAW);
    GLfloat* normalPtr = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    if (normalPtr) {
        memcpy(normalPtr, sphereNormals, sizeof(float) * numVertices * 3);
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
}
/**
 * Renders the core of the sun using a sphere with a smooth shading model.
 * The sphere is rendered using a series of triangle strips, with each strip
 * consisting of vertices from adjacent latitude lines.
 *
 * segments The number of segments to use when rendering the sphere.
 */
void renderSunCore(int segments) {
    glShadeModel(GL_SMOOTH);
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
}
/**
 * Simulates the effects of environmental lighting on the scene.
 * Adjusts the global ambient light based on the height of the light source.
 * Iterates through the pyramid instances and modifies their ambient and diffuse materials
 * based on the distance from the light source and the light's influence radius.
 *
 * The x-coordinate of the light source.
 * The y-coordinate of the light source.
 * The z-coordinate of the light source.
 * The radius of the light source's influence.
 */
void simulateEnvironmentalLightEffects(float lightX, float lightY, float lightZ, float lightRadius) {
    float influenceRadius = lightRadius * 5.0f;
    GLfloat globalAmbient[] = {
        0.2f + (lightY / 500.0f),
        0.2f + (lightY / 500.0f),
        0.1f + (lightY / 1000.0f),
        1.0f
    };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    float colorTemperature = (lightY > 250.0f) ? 6500.0f :
        (lightY > 100.0f) ? 3000.0f : 2000.0f;
    for (auto& pyramid : pyramidInstances) {
        float distance = sqrt(
            pow(pyramid.position.x - lightX, 2) +
            pow(pyramid.position.y - lightY, 2) +
            pow(pyramid.position.z - lightZ, 2)
        );
        if (distance < influenceRadius) {
            float intensity = 1.0f - (distance / influenceRadius);
            GLfloat dynamicAmbient[] = { 0.2f * intensity, 0.2f * intensity, 0.2f * intensity, 1.0f };
            GLfloat dynamicDiffuse[] = { intensity, intensity * 0.9f, intensity * 0.7f, 1.0f };

            glMaterialfv(GL_FRONT, GL_AMBIENT, dynamicAmbient);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, dynamicDiffuse);
        }
    }
}

/**
 * Initializes and manages the skybox's Vertex Buffer Objects.
 * Generates VBOs for vertex data and texture coordinates, then uploads the data to GPU memory.
 * Sets up texture bindings and vertex arrays for efficient rendering of all six skybox faces.
 */
void skyBoxVBO() {
    glGenBuffers(2, skyBoxVBOs);
    //upload vertex data using mapped buffer
    glBindBuffer(GL_ARRAY_BUFFER, skyBoxVBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), nullptr, GL_STATIC_DRAW);
    GLfloat* vertexPtr = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    if (vertexPtr) {
        memcpy(vertexPtr, vertexData.data(), vertexData.size() * sizeof(float));
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
    //upload texture coordinate data using mapped buffer
    glBindBuffer(GL_ARRAY_BUFFER, skyBoxVBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, textureCoordData.size() * sizeof(float), nullptr, GL_STATIC_DRAW);
    GLfloat* texCoordPtr = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    if (texCoordPtr) {
        memcpy(texCoordPtr, textureCoordData.data(), textureCoordData.size() * sizeof(float));
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
    //enable texture and vertex arrays
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    //set up vertex and texture coordinate pointers
    glBindBuffer(GL_ARRAY_BUFFER, skyBoxVBOs[0]);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, skyBoxVBOs[1]);
    glTexCoordPointer(2, GL_FLOAT, 0, 0);
    //draw front wall
    glBindTexture(GL_TEXTURE_2D, skyboxTextures.frontWall);
    glDrawArrays(GL_QUADS, 0, 4);
    //draw back wall
    glBindTexture(GL_TEXTURE_2D, skyboxTextures.backWall);
    glDrawArrays(GL_QUADS, 4, 4);
    //draw left wall
    glBindTexture(GL_TEXTURE_2D, skyboxTextures.leftWall);
    glDrawArrays(GL_QUADS, 8, 4);
    //draw right wall
    glBindTexture(GL_TEXTURE_2D, skyboxTextures.rightWall);
    glDrawArrays(GL_QUADS, 12, 4);
    //draw roof
    glBindTexture(GL_TEXTURE_2D, skyboxTextures.roof);
    glDrawArrays(GL_QUADS, 16, 4);
    //draw floor
    glBindTexture(GL_TEXTURE_2D, skyboxTextures.floor);
    glDrawArrays(GL_QUADS, 20, 4);
    // Cleanup
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
}
/**
 * Generates vertex and texture coordinate data for the skybox.
 * Takes parameters for overall size and grid detail. Updates vertex arrays for all six faces (front, back, left, right, roof, floor) with proper texture mapping coordinates.
 * The data is stored in vertexData and textureCoordData vectors.
 */
void skyBox(float size, int gridDivisions = 4) {
    float step = size / gridDivisions;
    vertexData.clear();
    textureCoordData.clear();
    Skybox::updateCubeVertices(size);
    //front wall
    for (size_t i = 0; i < 12; i += 3) {
        vertexData.push_back(Skybox::frontWallVertices[i]);
        vertexData.push_back(Skybox::frontWallVertices[i + 1]);
        vertexData.push_back(Skybox::frontWallVertices[i + 2]);
        float texU = (Skybox::frontWallVertices[i] + size / 2) / size;
        float texV = Skybox::frontWallVertices[i + 1] / size;
        textureCoordData.push_back(texU);
        textureCoordData.push_back(texV);
    }
    //back wall
    for (size_t i = 0; i < 12; i += 3) {
        vertexData.push_back(Skybox::backWallVertices[i]);
        vertexData.push_back(Skybox::backWallVertices[i + 1]);
        vertexData.push_back(Skybox::backWallVertices[i + 2]);
        float texU = (Skybox::backWallVertices[i] + size / 2) / size;
        float texV = Skybox::backWallVertices[i + 1] / size;
        textureCoordData.push_back(texU);
        textureCoordData.push_back(texV);
    }
    //left wall
    for (size_t i = 0; i < 12; i += 3) {
        vertexData.push_back(Skybox::leftWallVertices[i]);
        vertexData.push_back(Skybox::leftWallVertices[i + 1]);
        vertexData.push_back(Skybox::leftWallVertices[i + 2]);
        float texU = (Skybox::leftWallVertices[i + 2] + size / 2) / size;
        float texV = Skybox::leftWallVertices[i + 1] / size;
        textureCoordData.push_back(texU);
        textureCoordData.push_back(texV);
    }
    //right wall
    for (size_t i = 0; i < 12; i += 3) {
        vertexData.push_back(Skybox::rightWallVertices[i]);
        vertexData.push_back(Skybox::rightWallVertices[i + 1]);
        vertexData.push_back(Skybox::rightWallVertices[i + 2]);
        float texU = (Skybox::rightWallVertices[i + 2] + size / 2) / size;
        float texV = Skybox::rightWallVertices[i + 1] / size;
        textureCoordData.push_back(texU);
        textureCoordData.push_back(texV);
    }
    //roof
    for (size_t i = 0; i < 12; i += 3) {
        vertexData.push_back(Skybox::roofVertices[i]);
        vertexData.push_back(Skybox::roofVertices[i + 1]);
        vertexData.push_back(Skybox::roofVertices[i + 2]);
        float texU = (Skybox::roofVertices[i] + size / 2) / size;
        float texV = (Skybox::roofVertices[i + 2] + size / 2) / size;
        textureCoordData.push_back(texU);
        textureCoordData.push_back(texV);
    }
    //floor
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
/**
 * Handles the actual rendering of the skybox using the initialized VBOs.
 * Sets up lighting and materials, applies transformations, and renders each face with its corresponding texture.
 * Manages proper depth testing and polygon offset to avoid z-fighting issues.
 */
void renderSkyBox() {
    float position[3] = { 0.0f, 16.0f, 0.0f };
    float size = 10024.0f;
    int gridDivisions = 36;
    float halfSize = size / 2.0f;
    //enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    //setting up ambient light that affects the entire skybox
    GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat diffuseLight[] = { 0.7f, 0.7f, 0.7f, 1.0f };
    GLfloat lightPosition[] = { 0.0f, 1.0f, 0.0f, 0.0f }; //directional lighting x y z w
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    // Set global ambient light
    GLfloat globalAmbient[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    skyBox(size, gridDivisions);
    glPushMatrix();
    {
        glTranslatef(position[0], position[1], position[2]);
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f);
        skyBoxVBO();
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
    glPopMatrix();
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);
}


/**
 * Initializes the Vertex Buffer Objects (VBOs) used in the application.
 * This function sets up the VBOs for the pyramid, sun, and skybox geometry.
 * It also unbinds the buffer and texture objects, and marks the resources as initialized.
 * Finally, it logs a message indicating that the OpenGL resources have been initialized successfully.
 */
void initVBOs() {
    pyramidVBO();
    sunVBO();
    //renderSkyBox();
    //skyBoxVBO();

    // Unbind buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    resourcesInitialized = true;
    logMessage("OpenGL resources initialized successfully");
}

/**
 * Performs cleanup tasks for the OpenGL resources used in the application.
 * This function is responsible for deleting various VBO (Vertex Buffer Object) resources,
 * textures, and other OpenGL objects that were created during the initialization of the
 * application. It ensures that all resources are properly cleaned up before the application
 * exits.
 */
void cleanupVBO() {
    if (resourcesInitialized) {
        if (glutGetWindow()) {  // Ensure a valid OpenGL context
            // Cleanup sphere VBOs
            if (pyramidVBOs[0] || pyramidVBOs[1] || pyramidVBOs[2]) {// Cleanup pyramid VBOs
                glDeleteBuffers(3, pyramidVBOs);
                logMessage("Pyramid VBO resources successfully deleted.\n");
            }
            else {
                logMessage("Pyramid VBO resources were not initialized or already deleted.\n");
            }
            if (textures.pyramid) { // Cleanup pyramid texture
                glDeleteTextures(1, &textures.pyramid);
                logMessage("Pyramid texture successfully deleted.\n");
            }
            else {
                logMessage("Pyramid texture was not initialized or already deleted.\n");
            }
            if (sunVBOs[0] || sunVBOs[1] || sunVBOs[2]) {  // Cleanup sun VBOs
                glDeleteBuffers(3, sunVBOs);
                logMessage("Sun VBO resources successfully deleted.\n");
            }
            else {
                logMessage("Sun VBO resources were not initialized or already deleted.\n");
            }
            if (skyboxTextures.floor || skyboxTextures.frontWall || skyboxTextures.backWall || skyboxTextures.leftWall || skyboxTextures.rightWall || skyboxTextures.roof) { // Cleanup skybox textures
                GLuint textures[] = { skyboxTextures.floor, skyboxTextures.frontWall, skyboxTextures.backWall, skyboxTextures.leftWall, skyboxTextures.rightWall, skyboxTextures.roof };
                glDeleteTextures(6, textures);
                logMessage("Texture resources successfully deleted.\n");
            }
            else {
                logMessage("Texture resources were not initialized or already deleted.\n");
            }
            if (skyBoxVBOs[0] || skyBoxVBOs[1]) { // Cleanup skybox VBOs
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
/**
 * Performs cleanup tasks for the application, including:
 * - Cleaning up VBO resources
 * - Closes the OpenGL window
 * - Closing the log file
 */
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

/**
 * Handles mouse movement events for the camera.
 *
 * This function updates the camera's yaw and pitch angles based on the mouse movement.
 * It also ensures that the camera's pitch is constrained within a reasonable range to
 * prevent the camera from flipping.
 *
 *  x The current x-coordinate of the mouse cursor.
 *  y The current y-coordinate of the mouse cursor.
 */
void mouseMovement(int x, int y) {
    if (isPaused) return;
    if (firstMouse) {
        lastX = x;
        lastY = y;
        firstMouse = false;
        return;
    }
    //this calc the vp dimension
    int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
    int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);
    //calculates the mouse movement in the x and y direction depending on the vp
    int centerX = windowWidth / 2;
    int centerY = windowHeight / 2;
    float xoffset = x - lastX;
    float yoffset = lastY - y;
    //resets mouse position for next frame and prevents mouse jittering when mouse is on the edge of the screen
    int edgeThreshold = windowWidth / 5;
    if (x <= edgeThreshold || x >= (windowWidth - edgeThreshold) ||
        y <= edgeThreshold || y >= (windowHeight - edgeThreshold)) {
        glutWarpPointer(centerX, centerY);
        lastX = centerX;
        lastY = centerY;
        return;
    }
    lastX = x;
    lastY = y;
    //calculates the sensitivity of the mouse movement
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    //updates the camera's yaw and pitch angles
    yaw += xoffset;
    pitch += yoffset;
    //constrains the pitch to prevent camera flipping from a full 360 degree rotation in the y-axis or prevents barrel roll
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    updateCameraDirection();
}
void updateCameraDirection() {
    //for camera movement
    lookX = cos(yaw * M_PI / 180.0f) * cos(pitch * M_PI / 180.0f);
    lookY = sin(pitch * M_PI / 180.0f);
    lookZ = sin(yaw * M_PI / 180.0f) * cos(pitch * M_PI / 180.0f);
}
/**
 * Handles keyboard input events, including toggling the pause state, enabling/disabling fog, and toggling the HUD.
 *
 * When the ESC key is pressed, the application is paused or resumed. When paused, the cursor is set to the default and a message is logged. When resumed, the cursor is set to hidden and a message is logged.
 *
 * When the application is paused and the 'q' or 'Q' key is pressed, the application is exited.
 *
 * When the 'f' or 'F' key is pressed, fog is enabled or disabled. A message is logged to indicate the fog state.
 *
 * When the 'h' or 'H' key is pressed, the HUD is toggled on or off. A message is logged to indicate the HUD state.
 *
 * The ASCII character code of the key that was pressed.
 * The current x-coordinate of the mouse cursor.
 * The current y-coordinate of the mouse cursor.
 */
void keyboardDown(unsigned char key, int x, int y) {
    keyStates[key] = true;
    //toggle for pause and resume
    if (key == 27) { //ESC key
        if (!isPaused) {
            isPaused = true;
            glutSetCursor(GLUT_CURSOR_INHERIT);
            logMessage("Paused.");
        }
        else {
            isPaused = false;
            glutSetCursor(GLUT_CURSOR_NONE);
            logMessage("Resumed.");
        }
    }

    //exits only if 'q' or 'Q' is pressed and the system is paused
    if (isPaused && (key == 'q' || key == 'Q')) {
        logMessage("Application Exited by user.\n");
        cleanup();
        exit(0);
    }

    //toggle for fog
    if (key == 'f' || key == 'F') {
        fogEnabled = !fogEnabled;
        if (fogEnabled) {
            glEnable(GL_FOG);
            logMessage("Fog enabled.");
        }
        else {
            glDisable(GL_FOG);
            logMessage("Fog disabled.");
        }
    }
    //toggle for HUD
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
/**
 * Updates the player's movement based on the current key states.
 * This function is responsible for calculating the desired velocity and smoothly interpolating the current velocity towards the target velocity.
 * It also updates the camera position and Y-axis position based on the key states.
 */
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
void mouseWheel(int button, int dir, int x, int y) {
    if (dir > 0) {
        // Scroll up - increase scale
        scaling = min(scaling + 1.0f, 50.0f);
    }
    else {
        // Scroll down - decrease scale
        scaling = max(scaling - 1.0f, 3.0f);
    }
    glutPostRedisplay();
}
/**
 * Reshapes the OpenGL viewport and projection matrix to match the new window size.
 * This function is called whenever the window is resized.
 *
 * The new width of the window.
 * The new height of the window.
 */
void reshape(GLsizei width, GLsizei height) { //perspective projection
    if (height == 0) height = 1;
    float aspect = (float)width / (float)height;
    //viewport to the new window size
    glViewport(0, 0, width, height);
    //projection matrix mode
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //far plane increased to 10000 units (it renders this nth units)(frustrum culling) to see objects from greater distance
    gluPerspective(65.0f, aspect, 0.5f, 100000.0f);
    //enabled necessary rendering features
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL); // Less than or equal depth comparison for better depth precision
    //enabled line smoothing for better grid lines
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    //enabled blending for smoother lines
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //fog with distance fade and density
    if (fogEnabled) {
        GLfloat fogColor[4] = { 0.96f, 0.64f, 0.38f, 1.0f };
        glEnable(GL_FOG);
        glFogi(GL_FOG_MODE, GL_EXP2);
        glFogfv(GL_FOG_COLOR, fogColor);
        glFogf(GL_FOG_DENSITY, 0.00035f);
        glHint(GL_FOG_HINT, GL_NICEST);
    }
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
void renderFPS() {
    //calculate FPS
    frameCount++;
    auto currentTime = chrono::high_resolution_clock::now();
    chrono::duration<float> elapsedTime = currentTime - startTime;
    if (elapsedTime.count() >= 1.0f) {
        fps = frameCount / elapsedTime.count();
        frameCount = 0;
        startTime = currentTime;
    }
    //switched to a orthographic projection to render FPS
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, glutGet(GLUT_WINDOW_WIDTH), 0, glutGet(GLUT_WINDOW_HEIGHT));
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    //streaming frames
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
    if (!hudEnabled) return;  //renders if HUD is enabled
    // Set up orthographic projection for 2D overlay
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    gluOrtho2D(0, viewport[2], 0, viewport[3]);//this matches the vp
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    //hud coords
    glColor3f(1.0f, 1.0f, 1.0f);  // Set HUD text color to white
    int hudX = 10; //bottom left corner coords
    int hudY = 140;
    renderText(hudX, hudY, "HUD - Controls:");
    renderText(hudX, hudY - 20, "W/A/S/D: Move");
    renderText(hudX, hudY - 40, "Q/E: Move Up/Down");
    renderText(hudX, hudY - 60, "F: Toggle Fog");
    renderText(hudX, hudY - 80, "Scroll Wheel UP & DOWN: Pyramid Size Scaling");
    renderText(hudX, hudY - 100, "H: Toggle HUD");

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}
/**
 * Callback function for the timer event.
 * This function is called at regular intervals to update the movement of the scene and redraw the display.
 * It calls the `updateMovement()` function to update the movement of objects in the scene,
 * then calls `glutPostRedisplay()` to trigger a redraw of the display, and finally schedules the next timer callback
 * using `glutTimerFunc()`.
 *
 * Unused parameter passed to the timer callback function.
 */
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
    //render shapes
    renderPyramid();   //render pyramid
    renderSun(100.0f, segments, angleCircle, -2.0f, 3007.5f, 5.0f); //size, num of segments, angle of the rotation, and x y z coordinates
    renderSkyBox(); //skybox  // Render the complete cube map (floor, ceiling, and walls)
    //rendering hud elements
    renderPauseMenu(); // Render pause menu overlay if game is paused
    renderFPS(); //fps
    displayCoordinates(cameraX, cameraY, cameraZ); // Display the coordinates of the camera
    renderHUD(); //displays the HUD
    glutSwapBuffers();

    if (!isPaused) {
        anglePyramid += 0.2f;
        angleCube -= 0.15f;
        angleCircle += 0.2f;
    }
}