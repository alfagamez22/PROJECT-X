//particle system GLOBALS
// Random function helper with more granularity
float randomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}
float calculateSpeedForDepth(float depthFactor) {
    const float frontLayerSpeed = 150.0f; // Speed of the front layer
    const float backLayerSpeed = 150.0f; // Speed of the back layer
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


// //particle system
// //Set spawn area dimensions original
// void setSpawnArea(
//     float centerX,
//     float centerY,
//     float centerZ,
//     float rangeX = 20.0f,
//     float rangeY = 20.0f,
//     float rangeZ = 20.0f) {

//     spawnArea.centerX = centerX;
//     spawnArea.centerY = centerY;
//     spawnArea.centerZ = centerZ;
//     spawnArea.rangeX = rangeX;
//     spawnArea.rangeY = rangeY;
//     spawnArea.rangeZ = rangeZ;
// }

// void spawnParticle(float centerX = -1.0f, float centerY = -1.0f, float centerZ = -1.0f) {
//     if (particles.size() < maxParticles) {
//         Particle newParticle;

//         // Use provided coordinates or default spawn area
//         float spawnX = (centerX >= 0) ? centerX : spawnArea.centerX;
//         float spawnY = (centerY >= 0) ? centerY : spawnArea.centerY;
//         float spawnZ = (centerZ >= 0) ? centerZ : spawnArea.centerZ;

//         // Generate position within spawn area
//         newParticle.position[0] = randomFloat(spawnX - spawnArea.rangeX, spawnX + spawnArea.rangeX);
//         newParticle.position[1] = randomFloat(spawnY - spawnArea.rangeY, spawnY + spawnArea.rangeY);
//         newParticle.position[2] = randomFloat(spawnZ - spawnArea.rangeZ, spawnZ + spawnArea.rangeZ);

//         // Calculate depth factor based on spawn position
//         newParticle.depthFactor = (newParticle.position[2] - (spawnZ - spawnArea.rangeZ)) / (2 * spawnArea.rangeZ);

//         // Set velocities (primarily moving left)
//         float baseSpeed = calculateSpeedForDepth(newParticle.depthFactor);
//         newParticle.velocity[0] = -baseSpeed + randomFloat(-2.0f, 2.0f);
//         newParticle.velocity[1] = randomFloat(-2.0f, 2.0f) * newParticle.depthFactor;
//         newParticle.velocity[2] = randomFloat(-1.0f, 1.0f) * (1.0f - newParticle.depthFactor);

//         // Set visual properties
//         newParticle.size = calculateSizeForDepth(newParticle.depthFactor);
//         newParticle.isActive = true;

//         particles.push_back(newParticle);
//     }
// }
// void initializeParticles() {
//     // Initially populate the system with particles across the entire width
//     for (int i = 0; i < maxParticles; ++i) {
//         if (particles.size() < maxParticles) {
//             Particle newParticle;
//             newParticle.position[0] = randomFloat(leftBoundary, rightBoundary);
//             newParticle.position[1] = randomFloat(-verticalRange, verticalRange);
//             newParticle.position[2] = randomFloat(-depthRange, depthRange);

//             newParticle.velocity[0] = randomFloat(-15.0f, -10.0f);
//             newParticle.velocity[1] = randomFloat(-25.0f, -20.0f);
//             newParticle.velocity[2] = randomFloat(-35.0f, -30.0f);

//             newParticle.size = randomFloat(0.5f, 2.0f);
//             newParticle.isActive = true;

//             particles.push_back(newParticle);
//         }
//     }
// }
// void updateParticles() {
//     float respawnX = spawnArea.centerX;
//     float respawnRange = spawnArea.rangeX;

//     for (int i = 0; i < particles.size(); ++i) {
//         if (particles[i].isActive) {
//             // Update position
//             particles[i].position[0] += particles[i].velocity[0] * deltaT * speedMultiplier;
//             particles[i].position[1] += particles[i].velocity[1] * deltaT * speedMultiplier;
//             particles[i].position[2] += particles[i].velocity[2] * deltaT * speedMultiplier;

//             // Check if particle needs respawning
//             if (particles[i].position[0] < spawnArea.centerX - spawnArea.rangeX * 3) {
//                 // Respawn at original spawn area
//                 particles[i].position[0] = randomFloat(respawnX - respawnRange, respawnX + respawnRange);
//                 particles[i].position[1] = randomFloat(spawnArea.centerY - spawnArea.rangeY,
//                     spawnArea.centerY + spawnArea.rangeY);
//                 particles[i].position[2] = randomFloat(spawnArea.centerZ - spawnArea.rangeZ,
//                     spawnArea.centerZ + spawnArea.rangeZ);

//                 // Recalculate depth factor
//                 particles[i].depthFactor = (particles[i].position[2] - (spawnArea.centerZ - spawnArea.rangeZ))
//                     / (2 * spawnArea.rangeZ);

//                 // Reset velocities and properties
//                 float baseSpeed = calculateSpeedForDepth(particles[i].depthFactor);
//                 particles[i].velocity[0] = -baseSpeed + randomFloat(-2.0f, 2.0f);
//                 particles[i].velocity[1] = randomFloat(-2.0f, 2.0f) * particles[i].depthFactor;
//                 particles[i].velocity[2] = randomFloat(-1.0f, 1.0f) * (1.0f - particles[i].depthFactor);

//                 particles[i].size = calculateSizeForDepth(particles[i].depthFactor);
//             }

//             // Keep particles within vertical and depth bounds
//             float verticalBound = spawnArea.rangeY * 2;
//             float depthBound = spawnArea.rangeZ * 2;

//             if (abs(particles[i].position[1] - spawnArea.centerY) > verticalBound) {
//                 particles[i].velocity[1] *= -0.5f;
//             }
//             if (abs(particles[i].position[2] - spawnArea.centerZ) > depthBound) {
//                 particles[i].velocity[2] *= -0.5f;
//             }
//         }
//     }
// }
// void renderParticles() {
//     for (int i = 0; i < particles.size(); ++i) {
//         if (particles[i].isActive) {
//             // Render each particle as a small sphere (use your existing render function) tis part we render our spheres
//             (1.0f, segments, 0.0f, particles[i].position[0], particles[i].position[1], particles[i].position[2]);
//         }
//     }
// }
// void update() {
//     // Spawn new particles if needed
//     if (particles.size() < maxParticles) {
//         spawnParticle();
//     }
//     updateParticles();
//     renderParticles();
// }


//from display

  // updateParticles();  // display particle system
    // renderParticles(); // display particle system
    //update(); //keeps updating to spawn particles