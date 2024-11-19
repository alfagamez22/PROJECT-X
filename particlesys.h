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
    const float minSize = 20.1f; // Minimum size of the particles
    const float maxSize = 20.1f; // Maximum size of the particles
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

