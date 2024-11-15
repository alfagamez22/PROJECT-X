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

#include <fstream>
#include <ctime>
#include <direct.h> //for _getcwd
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


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
void updateParticles();
void renderParticles();
void displayCoordinates(float coordX, float coordY, float coordZ);
void renderHUD();

void initVBOs(); // Initialize VBOs
void cleanupVBO(); // Cleanup VBOs
void cleanup();

GLuint loadTexture(const char* filename); // Load texture from file

/* Global variables */
// Angle of rotation for shapes
float anglePyramid = 0.0f;
float angleCube = 0.0f;
float angleCircle = 0.0f;  // Rotation angle for the circle

int refreshMills = 5; // Refresh interval in milliseconds the lower the better maximum 5 minimum 25 (15 best) to avoid visual bugs

//Camera Dimensions
// Camera position and orientation starting
float cameraX = 0.0f;
float cameraY = 17.5f;
float cameraZ = 10.0f;
float cameraSpeed = 0.1f; //can be adjusted the higher the faster 0.1 is the best it was 0.3 kinda quick
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
float maxVelocity = 0.15f;
float acceleration = 0.05f; //inertia
float deceleration = 0.1f; //inertia

// Window state
bool isPaused = false;
bool firstMouse = true;
//Draw fps
chrono::time_point<chrono::high_resolution_clock> startTime;
int frameCount = 0;
float fps = 0.0f;
//Hud display
bool hudEnabled = false;

//number of segments for the sphere (higher is laggier) but may bug out 20 is best and smooth circle
const int segments = 10;

//particle system
// Random function helper with more granularity
float randomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}
float calculateSpeedForDepth(float depthFactor) {
    const float frontLayerSpeed = 15.0f; // Speed of the front layer
    const float backLayerSpeed = 15.0f; // Speed of the back layer
    return backLayerSpeed + (frontLayerSpeed - backLayerSpeed) * depthFactor;
}
float calculateSizeForDepth(float depthFactor) {
    const float minSize = 5.0f; // Minimum size of the particles
    const float maxSize = 5.0f; // Maximum size of the particles
    return minSize + (maxSize - minSize) * depthFactor;
}
// Added spawn boundaries for the desert storm effect
const float rightBoundary = 10.0f;   // Where particles spawn 300
const float leftBoundary = 10.0f;   // Where particles get recycled -300
const float verticalRange = 10.0f;    // Height range for particles 300
const float depthRange = 50.0f;       // Depth range for particles when lower depth fps dips 50
//const float particleSize = 0.5f;      // Size of the particles
const int maxParticles = 300; // Max particles in the system
//const float gravity = -9.81f; // Gravity constant (negative to pull downward) good for rain particle system for later
const float pi = M_PI;  // Using the irrational value of pi for randomness
float deltaT = 0.016f; // Time step (assuming 60 FPS)
//speed of the particles multiplier
float speedMultiplier = 1.01f;  // Adjust this to control particle speed (1.0 default speed kinda fast but good for desert scene)
// Particle struct and list to store particles
struct Particle {
    float position[3];
    float velocity[3];
    float size;        // Added variable size for more realistic effect
    float depthFactor;
    bool isActive;
};
// Spawn area control
struct SpawnArea {
    float centerX = 300.0f;    // Center X position of spawn area
    float centerY = 35.0f;    // Center Y position of spawn area 20 default
    float centerZ = 0.0f;    // Center Z position of spawn area
    float rangeX = 250.0f;     // Spawn range in X direction 300
    float rangeY = 10.0f;     // Spawn range in Y direction 100
    float rangeZ = 500.0f;     // Spawn range in Z direction 300
};
// Default spawn area
SpawnArea spawnArea;
vector<Particle> particles;

//for logging purposes
bool resourcesInitialized = false;
ofstream logFile;
string logPath;


//Initialize VBOs
GLuint pyramidVBOs[2], sphere3DVBOs[2], cubeVBOs[2];
GLuint textureID;

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


// Vertex and texture arrays for the cube
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


//Texture Loader
GLuint loadTexture(const char* filename) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture wrapping/filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Load image using stb_image
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        printf("Failed to load texture\n");
    }

    stbi_image_free(data);
    return textureID;
}

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

    textureID = loadTexture("c:/Users/buanh/Documents/VSCODE_SAVES/OpenGL/Projects/3D_WORLD_BUAN/doggo.jpg");  // Replace with your texture file

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
                sphereColors[index * 3 + 1] = 0.0f;  // Green component
                sphereColors[index * 3 + 2] = 0.0f;  // Blue component
            }
            else if (lon % 3 == 1) {
                sphereColors[index * 3] = 0.0f;  // Red component
                sphereColors[index * 3 + 1] = 1.0f;  // Green component (Yellow)
                sphereColors[index * 3 + 2] = 0.0f;  // Blue component
            }
            else {
                sphereColors[index * 3] = 0.0f;  // Red component
                sphereColors[index * 3 + 1] = 0.0f;  // Green component
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

void initVBOs() {
    cubeVBO();
    pyramidVBO();
    sphere3dVBO();

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
            newParticle.velocity[1] = randomFloat(-15.0f, -10.0f);
            newParticle.velocity[2] = randomFloat(-15.0f, -10.0f);

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

void renderCubeWithGrid(float size, int gridDivisions = 4) {
    float halfSize = size / 2.0f;
    float step = size / gridDivisions;

    // Define vertices using glm::vec3
    vector<glm::vec3> vertices = {
        // Front face
        glm::vec3(-halfSize, -halfSize,  halfSize),
        glm::vec3(halfSize, -halfSize,  halfSize),
        glm::vec3(halfSize,  halfSize,  halfSize),
        glm::vec3(-halfSize,  halfSize,  halfSize),

        // Back face
        glm::vec3(-halfSize, -halfSize, -halfSize),
        glm::vec3(-halfSize,  halfSize, -halfSize),
        glm::vec3(halfSize,  halfSize, -halfSize),
        glm::vec3(halfSize, -halfSize, -halfSize),

        // Left face
        glm::vec3(-halfSize, -halfSize, -halfSize),
        glm::vec3(-halfSize, -halfSize,  halfSize),
        glm::vec3(-halfSize,  halfSize,  halfSize),
        glm::vec3(-halfSize,  halfSize, -halfSize),

        // Right face
        glm::vec3(halfSize, -halfSize, -halfSize),
        glm::vec3(halfSize,  halfSize, -halfSize),
        glm::vec3(halfSize,  halfSize,  halfSize),
        glm::vec3(halfSize, -halfSize,  halfSize),

        // Top face
        glm::vec3(-halfSize, halfSize, -halfSize),
        glm::vec3(-halfSize, halfSize,  halfSize),
        glm::vec3(halfSize, halfSize,  halfSize),
        glm::vec3(halfSize, halfSize, -halfSize),

        // Bottom face
        glm::vec3(-halfSize, -halfSize, -halfSize),
        glm::vec3(halfSize, -halfSize, -halfSize),
        glm::vec3(halfSize, -halfSize,  halfSize),
        glm::vec3(-halfSize, -halfSize,  halfSize),

        // Floor vertices
        glm::vec3(-halfSize, 0.0f, -halfSize),
        glm::vec3(halfSize, 0.0f, -halfSize),
        glm::vec3(halfSize, 0.0f, halfSize),
        glm::vec3(-halfSize, 0.0f, halfSize)
    };

    // Render solid cube
    glBegin(GL_QUADS);
    glColor3f(0.5f, 0.5f, 0.5f);

    for (const auto& vertex : vertices) {
        glVertex3f(vertex.x, vertex.y, vertex.z);
    }
    glEnd();

    // Render grid lines
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f);  // Black grid lines

    // Generate grid lines for each face
    for (int face = 0; face < 7; face++) {
        glm::vec3 origin, dirX, dirY;

        switch (face) {
        case 0: // Front
            origin = glm::vec3(-halfSize, -halfSize, halfSize);
            dirX = glm::vec3(1, 0, 0);
            dirY = glm::vec3(0, 1, 0);
            break;
        case 1: // Back
            origin = glm::vec3(-halfSize, -halfSize, -halfSize);
            dirX = glm::vec3(1, 0, 0);
            dirY = glm::vec3(0, 1, 0);
            break;
        case 2: // Left
            origin = glm::vec3(-halfSize, -halfSize, -halfSize);
            dirX = glm::vec3(0, 0, 1);
            dirY = glm::vec3(0, 1, 0);
            break;
        case 3: // Right
            origin = glm::vec3(halfSize, -halfSize, -halfSize);
            dirX = glm::vec3(0, 0, 1);
            dirY = glm::vec3(0, 1, 0);
            break;
        case 4: // Top
            origin = glm::vec3(-halfSize, halfSize, -halfSize);
            dirX = glm::vec3(1, 0, 0);
            dirY = glm::vec3(0, 0, 1);
            break;
        case 5: // Bottom
            origin = glm::vec3(-halfSize, -halfSize, -halfSize);
            dirX = glm::vec3(1, 0, 0);
            dirY = glm::vec3(0, 0, 1);
            break;
        case 6: // Floor
            origin = glm::vec3(-halfSize, 0.0f, -halfSize);
            dirX = glm::vec3(1, 0, 0);
            dirY = glm::vec3(0, 0, 1);
            break;
        }

        // Draw grid lines
        for (float i = 0; i <= gridDivisions; i++) {
            float t = i * step;
            glm::vec3 start = origin + dirX * t;
            glm::vec3 end = start + dirY * size;
            glVertex3f(start.x, start.y, start.z);
            glVertex3f(end.x, end.y, end.z);

            start = origin + dirY * t;
            end = start + dirX * size;
            glVertex3f(start.x, start.y, start.z);
            glVertex3f(end.x, end.y, end.z);
        }
    }
    glEnd();
}

void renderCubeMap() {
    // Position using GLM vectors
    glm::vec3 position(0.0f, 16.0f, 0.0f);
    float size = 1024.0f;
    int gridDivisions = 36;

    glPushMatrix();
    {
        // Create transformation matrix
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);

        // Optional rotation
        // model = glm::rotate(model, glm::radians(rotationAngle), glm::vec3(0.1f, 1.0f, 0.1f));

        // Apply transformation
        glMultMatrixf(glm::value_ptr(model));

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f);

        renderCubeWithGrid(size, gridDivisions);

        glDisable(GL_POLYGON_OFFSET_FILL);
    }
    glPopMatrix();
}


void timer(int value) {
    updateMovement();
    glutPostRedisplay();
    glutTimerFunc(refreshMills, timer, 0);
}

//perspective projection
void reshape(GLsizei width, GLsizei height) {
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
    glFogf(GL_FOG_START, 3005.0f);     // Start fog after 2000 units 35 best nearest
    glFogf(GL_FOG_END, 5000.0f);       // Full fog by 4500 units 50 or100 for farthest
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
    renderSphere3D(1.0f, segments, angleCircle, -2.0f, 17.5f, 5.0f); //this is vbo
    renderAnotherSphere3D(1.0f, 10, angleCircle, 5.0f, 17.5f, 5.0f); //no vbo

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




/*

smooth sphere without using VBO

void renderAnotherSphere3D(float radius, int segments, float angle, float xPos, float yPos, float zPos) {
    // Arrays for vertices and colors
    vector<float> sphereVertices;
    vector<float> sphereColors;

    // Generate vertices and colors
    for (int lat = 0; lat <= segments; ++lat) {
        float theta = lat * M_PI / segments; // Latitude angle (0 to pi)
        for (int lon = 0; lon <= segments; ++lon) {
            float phi = lon * 2.0f * M_PI / segments; // Longitude angle (0 to 2pi)

            // Vertex positions
            sphereVertices.push_back(radius * sin(theta) * cos(phi)); // x
            sphereVertices.push_back(radius * sin(theta) * sin(phi)); // y
            sphereVertices.push_back(radius * cos(theta));           // z

            // Vertex colors (simple gradient)
            sphereColors.push_back((float)lat / segments); // Red
            sphereColors.push_back((float)lon / segments); // Green
            sphereColors.push_back(0.9f);                 // Blue
        }
    }

    glPushMatrix();
    glTranslatef(xPos, yPos, zPos); // Position the sphere
    glRotatef(angle, 1.0f, 1.0f, 0.0f); // Rotate for animation

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, sphereVertices.data());
    glColorPointer(3, GL_FLOAT, 0, sphereColors.data());

    // Render the sphere using triangle strips
    for (int lat = 0; lat < segments; ++lat) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int lon = 0; lon <= segments; ++lon) {
            int curr = lat * (segments + 1) + lon;
            int next = (lat + 1) * (segments + 1) + lon;

            glColor3fv(&sphereColors[curr * 3]);
            glVertex3fv(&sphereVertices[curr * 3]);

            glColor3fv(&sphereColors[next * 3]);
            glVertex3fv(&sphereVertices[next * 3]);
        }
        glEnd();
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glPopMatrix();
}


// Function to render a cube with grid lines on each face
void  renderCubeWithGrid(float size, int gridDivisions = 4) {
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

    // Define the four corners of the floor (planar square)
    glVertex3f(-halfSize, 0.0f, -halfSize);
    glVertex3f(halfSize, 0.0f, -halfSize);
    glVertex3f(halfSize, 0.0f, halfSize);
    glVertex3f(-halfSize, 0.0f, halfSize);

    glEnd();

    // Overlay grid lines on each face
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f);  // Set grid color to black
    // Front face grid
    for (float i = -halfSize; i <= halfSize; i += step) {
        glVertex3f(i, -halfSize, halfSize);
        glVertex3f(i, halfSize, halfSize);  // Vertical lines
        glVertex3f(-halfSize, i, halfSize);
        glVertex3f(halfSize, i, halfSize);  // Horizontal lines
    }

    // Back face grid
    for (float i = -halfSize; i <= halfSize; i += step) {
        glVertex3f(i, -halfSize, -halfSize);
        glVertex3f(i, halfSize, -halfSize);
        glVertex3f(-halfSize, i, -halfSize);
        glVertex3f(halfSize, i, -halfSize);
    }

    // Left face grid
    for (float i = -halfSize; i <= halfSize; i += step) {
        glVertex3f(-halfSize, -halfSize, i);
        glVertex3f(-halfSize, halfSize, i);
        glVertex3f(-halfSize, i, -halfSize);
        glVertex3f(-halfSize, i, halfSize);
    }

    // Right face grid
    for (float i = -halfSize; i <= halfSize; i += step) {
        glVertex3f(halfSize, -halfSize, i);
        glVertex3f(halfSize, halfSize, i);
        glVertex3f(halfSize, i, -halfSize);
        glVertex3f(halfSize, i, halfSize);
    }

    // Top face grid
    for (float i = -halfSize; i <= halfSize; i += step) {
        glVertex3f(i, halfSize, -halfSize);
        glVertex3f(i, halfSize, halfSize);
        glVertex3f(-halfSize, halfSize, i);
        glVertex3f(halfSize, halfSize, i);
    }

    // Bottom face grid
    for (float i = -halfSize; i <= halfSize; i += step) {
        glVertex3f(i, -halfSize, -halfSize);
        glVertex3f(i, -halfSize, halfSize);
        glVertex3f(-halfSize, -halfSize, i);
        glVertex3f(halfSize, -halfSize, i);
    }

    glEnd();

    // Overlay grid lines on the floor
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 0.0f);  // Set grid color to black
    // Floor grid at y = 0
    for (float i = -halfSize; i <= halfSize; i += step) {
        // Vertical lines along x-axis
        glVertex3f(i, 0.0f, -halfSize);
        glVertex3f(i, 0.0f, halfSize);

        // Horizontal lines along z-axis
        glVertex3f(-halfSize, 0.0f, i);
        glVertex3f(halfSize, 0.0f, i);
    }
    glEnd();
}

// Modified renderCubeMap function with additional features
void renderCubeMap() {
    //static float rotationAngle = 0.0f;  // For rotation animation if desired

    // Set adjustable position coordinates
    float x = 0.0f;
    float y = 16.0f;
    float z = 0.0f;

    float size = 1024.0f; //1024 best for skybox 200 for experimental
    int gridDivisions = 36;

    glPushMatrix();
    {
        // Position the cube
        glTranslatef(x, y, z);

        // Optional: Add rotation for better visualization
        //glRotatef(rotationAngle, 0.1f, 1.0f, 0.1f);

        // Enable polygon offset for better grid line rendering
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(1.0f, 1.0f);

        // Render the cube with grid
        renderCubeWithGrid(size, gridDivisions);
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
    glPopMatrix();

    // Uncomment to add rotation animation
    //rotationAngle += 0.1f;
}



*/