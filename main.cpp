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

// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtc/type_ptr.hpp>

#include "textureloader.cpp" //texture method
#include "interaction.h" //movement globals
#include "particlesys.h" //particle system global var
#include "cube.h" //cube coordinates
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
void renderCube();
void renderPyramid();
void renderSphere3D(float radius, int segments, float angle, float xPos, float yPos, float zPos);
void renderCubeMap();
void updateParticles();
void renderParticles();
void displayCoordinates(float coordX, float coordY, float coordZ);
void renderHUD();

void initVBOs(); // Initialize VBOs
void cleanupVBO(); // Cleanup VBOs
void cleanup();

GLuint loadTexture(const char* filename); // Load texture from file

//Initialize VBOs
GLuint pyramidVBOs[2], sphere3DVBOs[2], cubeVBOs[2], renderCubeWithGridVBOs[4];
GLuint textureID;

/* Global variables */
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
float lightSpeed = 0.05f;
int currentCorner = 0;

//CubeGrid perlin noise variable
float persistence = 0.5f; // Controls detail persistence across octaves
int octaves = 6; // Number of noise layers (more = more detail)
float frequency = 0.02f; // Base frequency of the noise


//for logging purposes
bool resourcesInitialized = false;
ofstream logFile;
string logPath;

vector<float> vertexData;
vector<float> cubeColorData;

//particle system
vector<Particle> particles;

//texture coordinate data
vector<float> textureCoordData;

class PerlinNoise {
private:
    std::vector<int> p;

public:
    PerlinNoise() {
        p.resize(512);
        for (int i = 0; i < 256; i++) p[i] = i;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(p.begin(), p.begin() + 256, gen);
        for (int i = 0; i < 256; i++) p[256 + i] = p[i];
    }

    double noise(double x, double y, double z) {
        int X = (int)floor(x) & 255;
        int Y = (int)floor(y) & 255;
        int Z = (int)floor(z) & 255;

        x -= floor(x);
        y -= floor(y);
        z -= floor(z);

        double u = fade(x);
        double v = fade(y);
        double w = fade(z);

        int A = p[X] + Y;
        int AA = p[A] + Z;
        int AB = p[A + 1] + Z;
        int B = p[X + 1] + Y;
        int BA = p[B] + Z;
        int BB = p[B + 1] + Z;

        return lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z),
            grad(p[BA], x - 1, y, z)),
            lerp(u, grad(p[AB], x, y - 1, z),
                grad(p[BB], x - 1, y - 1, z))),
            lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1),
                grad(p[BA + 1], x - 1, y, z - 1)),
                lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
                    grad(p[BB + 1], x - 1, y - 1, z - 1))));
    }

private:
    double fade(double t) { return t * t * t * (t * (t * 6 - 15) + 10); }
    double lerp(double t, double a, double b) { return a + t * (b - a); }
    double grad(int hash, double x, double y, double z) {
        int h = hash & 15;
        double u = h < 8 ? x : y;
        double v = h < 4 ? y : h == 12 || h == 14 ? x : z;
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }
};
// Add this as a global variable
PerlinNoise perlin;

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

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glutSetCursor(GLUT_CURSOR_NONE);
    initVBOs();

    // Start the clock on startup from 0 to -> calculate FPS
    startTime = chrono::high_resolution_clock::now();
    logMessage("Application starting Now...");

    // Enable texturing
    glEnable(GL_TEXTURE_2D);

    textureID = loadTexture("c:/Users/buanh/Documents/VSCODE_SAVES/OpenGL/Projects/3D_WORLD_BUAN/sand.png");  // Replace with your texture file

    atexit(cleanup);

}

void renderCube() {
    glPushMatrix();

    // Enable lighting for this specific object
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Configure light properties
    GLfloat lightPos[] = { 0.0f, 5.0f, 10.0f, 1.0f }; // Position of the light
    GLfloat lightAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f }; // Soft white ambient light
    GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // Bright white diffuse light
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // Bright white specular light

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    // Configure material properties for brightness
    GLfloat matAmbient[] = { 1.7f, 1.7f, 1.7f, 1.0f }; // Brighter ambient reflection
    GLfloat matDiffuse[] = { 0.9f, 0.9f, 0.9f, 1.0f }; // Brighter diffuse reflection
    GLfloat matSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f }; // Strong specular reflection
    GLfloat matShininess[] = { 100.0f };               // High shininess for glossy effect

    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

    // Position the cube in 3D space
    glTranslatef(1.5f, 18.0f, -7.0f);
    glRotatef(angleCube, 1.0f, 1.0f, 1.0f);

    glBindTexture(GL_TEXTURE_2D, textureID);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    // Bind vertex VBO
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBOs[0]);
    glVertexPointer(3, GL_FLOAT, 0, nullptr);

    // Bind texture VBO
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBOs[1]);
    glTexCoordPointer(2, GL_FLOAT, 0, nullptr);

    glDrawArrays(GL_QUADS, 0, 24);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Disable lighting after rendering the cube
    glDisable(GL_LIGHTING);
    glDisable(GL_LIGHT0);

    glPopMatrix();
}
void cubeVBO() {
    glGenBuffers(2, cubeVBOs);

    // Vertex coordinates
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // Texture coordinates
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoords), textureCoords, GL_STATIC_DRAW);
}

void renderPyramid() {
    glPushMatrix();
    // Position the pyramid in 3D space
    glTranslatef(-1.5f, 18.0f, -6.0f);
    // Rotate the pyramid  coordinate system only rotating in y axis
    glRotatef(anglePyramid, 0.0f, 1.0f, 0.0f);

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

    glPopMatrix();
}\
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

void renderCubeWithGridVBO() {
    // Generate VBO IDs (now 3 for vertex, texture coords, and noise)
    glGenBuffers(3, renderCubeWithGridVBOs);

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, renderCubeWithGridVBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    // Upload texture coordinate data
    glBindBuffer(GL_ARRAY_BUFFER, renderCubeWithGridVBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, textureCoordData.size() * sizeof(float), textureCoordData.data(), GL_STATIC_DRAW);

    // Enable texture
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Render textured cube
    glBindBuffer(GL_ARRAY_BUFFER, renderCubeWithGridVBOs[0]);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, renderCubeWithGridVBOs[1]);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, 0);

    glDrawArrays(GL_QUADS, 0, vertexData.size() / 3);

    // Cleanup
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
}

// void renderCubeWithGridVBO() {
//     // Generate VBO IDs
//     glGenBuffers(2, renderCubeWithGridVBOs);

//     // Upload cube vertex data
//     glBindBuffer(GL_ARRAY_BUFFER, renderCubeWithGridVBOs[0]);
//     glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

//     // Upload cube color data
//     glBindBuffer(GL_ARRAY_BUFFER, renderCubeWithGridVBOs[1]);
//     glBufferData(GL_ARRAY_BUFFER, cubeColorData.size() * sizeof(float), cubeColorData.data(), GL_STATIC_DRAW);

//     // Render solid cube
//     glBindBuffer(GL_ARRAY_BUFFER, renderCubeWithGridVBOs[0]);
//     glEnableClientState(GL_VERTEX_ARRAY);
//     glVertexPointer(3, GL_FLOAT, 0, 0);

//     glBindBuffer(GL_ARRAY_BUFFER, renderCubeWithGridVBOs[1]);
//     glEnableClientState(GL_COLOR_ARRAY);
//     glColorPointer(3, GL_FLOAT, 0, 0);

//     glDrawArrays(GL_QUADS, 0, vertexData.size() / 3);

//     // Cleanup
//     glDisableClientState(GL_VERTEX_ARRAY);
//     glDisableClientState(GL_COLOR_ARRAY);
//     //glBindBuffer(GL_ARRAY_BUFFER, 0);
// }

// Function to generate and update vertices

// void renderCubeWithGrid(float size, int gridDivisions = 4) {
//     float step = size / gridDivisions;

//     vertexData.clear();
//     cubeColorData.clear();
//     Skybox::updateCubeVertices(size);
//     // Use Skybox::globalCubeVertices to access the vertices
//     for (int i = 0; i < Skybox::globalCubeVertices.size(); i += 3) {
//         vertexData.push_back(Skybox::globalCubeVertices[i]);
//         vertexData.push_back(Skybox::globalCubeVertices[i + 1]);
//         vertexData.push_back(Skybox::globalCubeVertices[i + 2]);

//         // cubeColorData.push_back(0.8f); //R
//         // cubeColorData.push_back(0.8f); //G
//         // cubeColorData.push_back(0.8f); //B
//         // cubeColorData.push_back(1.0);

//         cubeColorData.push_back(0.96f); //R
//         cubeColorData.push_back(0.64f); //G
//         cubeColorData.push_back(0.38f); //B
//         cubeColorData.push_back(0.0);

//         //0.96f, 0.64f, 0.38f, 1.0f
//     }
// }

// void renderCubeWithGrid(float size, int gridDivisions = 4) {
//     float step = size / gridDivisions;
//     vertexData.clear();
//     cubeColorData.clear();

//     Skybox::updateCubeVertices(size);

//     for (int i = 0; i < Skybox::globalCubeVertices.size(); i += 3) {
//         float x = Skybox::globalCubeVertices[i];
//         float y = Skybox::globalCubeVertices[i + 1];
//         float z = Skybox::globalCubeVertices[i + 2];

//         vertexData.push_back(x);
//         vertexData.push_back(y);
//         vertexData.push_back(z);

//         // Generate Perlin noise value
//         float noiseValue = 0.0f;
//         float amplitude = 1.0f;
//         float totalAmplitude = 0.0f;

//         for (int oct = 0; oct < octaves; oct++) {
//             float freq = frequency * pow(2, oct);
//             float perlinValue = perlin.noise(x * freq, y * freq, z * freq);
//             noiseValue += perlinValue * amplitude;
//             totalAmplitude += amplitude;
//             amplitude *= persistence;
//         }

//         noiseValue /= totalAmplitude;
//         noiseValue = (noiseValue + 1.0f) * 0.5f;  // Normalize to 0-1

//         // Desert sand colors with noise variation
//        // Use noise value to modify base sand color
//         cubeColorData.push_back(0.95f * (0.8f + 0.2f * noiseValue));  // R
//         cubeColorData.push_back(0.87f * (0.8f + 0.2f * noiseValue));  // G
//         cubeColorData.push_back(0.60f * (0.8f + 0.2f * noiseValue));  // B
//         cubeColorData.push_back(1.0f);  // A
//     }
// }

// void renderCubeWithGrid(float size, int gridDivisions = 4) {
//     float step = size / gridDivisions;
//     vertexData.clear();
//     textureCoordData.clear();

//     Skybox::updateCubeVertices(size);

//     for (int i = 0; i < Skybox::globalCubeVertices.size(); i += 12) {
//         for (int j = 0; j < 4; j++) {
//             float x = Skybox::globalCubeVertices[i + j * 3];
//             float y = Skybox::globalCubeVertices[i + j * 3 + 1];
//             float z = Skybox::globalCubeVertices[i + j * 3 + 2];

//             // Generate Perlin noise value
//             float noiseValue = 0.0f;
//             float amplitude = 1.0f;
//             float totalAmplitude = 0.0f;

//             for (int oct = 0; oct < octaves; oct++) {
//                 float freq = frequency * pow(2, oct);
//                 float perlinValue = perlin.noise(x * freq, y * freq, z * freq);
//                 noiseValue += perlinValue * amplitude;
//                 totalAmplitude += amplitude;
//                 amplitude *= persistence;
//             }

//             noiseValue /= totalAmplitude;
//             noiseValue = (noiseValue + 1.0f) * 0.5f;  // Normalize to 0-1

//             vertexData.push_back(x);
//             vertexData.push_back(y);
//             vertexData.push_back(z);

//             // Add texture coordinates with noise influence
//             float texU = (float)(j % 2) + noiseValue * 0.1f;
//             float texV = (float)(j / 2) + noiseValue * 0.1f;
//             textureCoordData.push_back(texU);
//             textureCoordData.push_back(texV);
//         }
//     }
// }

void renderCubeWithGrid(float size, int gridDivisions = 4) {
    float step = size / gridDivisions;
    vertexData.clear();
    textureCoordData.clear();

    Skybox::updateCubeVertices(size);

    for (int i = 0; i < Skybox::globalCubeVertices.size(); i += 12) {
        for (int j = 0; j < 4; j++) {
            float x = Skybox::globalCubeVertices[i + j * 3];
            float y = Skybox::globalCubeVertices[i + j * 3 + 1];
            float z = Skybox::globalCubeVertices[i + j * 3 + 2];

            vertexData.push_back(x);
            vertexData.push_back(y);
            vertexData.push_back(z);

            // Check if these vertices are part of the floor (y = 0)
            if (y == 0.0f) {
                // Add texture coordinates for floor
                float texU = (x + size / 2) / size;  // Normalize x coordinate
                float texV = (z + size / 2) / size;  // Normalize z coordinate
                textureCoordData.push_back(texU);
                textureCoordData.push_back(texV);
            }
            else {
                // Add dummy texture coordinates for non-floor vertices
                textureCoordData.push_back(1.0f);
                textureCoordData.push_back(1.0f);

            }
        }
    }
}



void renderCubeMap() {
    float position[3] = { 0.0f, 16.0f, 0.0f };
    float size = 524.0f;
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

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

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

    renderCubeWithGrid(size, gridDivisions);

    glPushMatrix();
    {
        glTranslatef(position[0], position[1], position[2]);

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f);

        renderCubeWithGridVBO();

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
    cubeVBO();
    pyramidVBO();
    sphere3dVBO();
    renderCubeWithGridVBO();

    // Unbind buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    resourcesInitialized = true;
    logMessage("OpenGL resources initialized successfully");
}

void cleanupVBO() {
    if (resourcesInitialized) {
        if (glutGetWindow()) {  // Ensure a valid OpenGL context
            // Cleanup cube VBOs
            if (cubeVBOs[0] || cubeVBOs[1]) {
                glDeleteBuffers(2, cubeVBOs);
                logMessage("Cube VBO resources successfully deleted.\n");
            }
            else {
                logMessage("Cube VBO resources were not initialized or already deleted.\n");
            }
            // Cleanup pyramid VBOs
            if (pyramidVBOs[0] || pyramidVBOs[1]) {
                glDeleteBuffers(2, pyramidVBOs);
                logMessage("Pyramid VBO resources successfully deleted.\n");
            }
            else {
                logMessage("Pyramid VBO resources were not initialized or already deleted.\n");
            }
            // Cleanup sphere VBOs
            if (sphere3DVBOs[0] || sphere3DVBOs[1]) {
                glDeleteBuffers(2, sphere3DVBOs);
                logMessage("Sphere VBO resources successfully deleted.\n");
            }
            else {
                logMessage("Sphere VBO resources were not initialized or already deleted.\n");
            }
            if (textureID) {
                glDeleteTextures(1, &textureID);
                logMessage("Texture resources successfully deleted.\n");
            }
            else {
                logMessage("Texture resources were not initialized or already deleted.\n");
            }
            if (renderCubeWithGridVBOs[0] || renderCubeWithGridVBOs[1]) {
                glDeleteBuffers(2, renderCubeWithGridVBOs);
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
    gluPerspective(60.0f, aspect, 0.5f, 10000.0f);

    // Enable necessary rendering features
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL); // Less than or equal depth comparison for better depth precision

    //Enable back face culling for better cube rendering (frustrum culling)
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    //glCullFace(GL_FRONT);

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
    glFogf(GL_FOG_START, 1000.0f);     // Start fog after 2000 units 35 best nearest
    glFogf(GL_FOG_END, 35000.0f);       // Full fog by 4500 units 50 or100 for farthest
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
    renderCubeMap(); //skybox  // Render the complete cube map (floor, ceiling, and walls)
    // Render shapes
    renderCube();         // Only render cube once
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