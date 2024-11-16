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