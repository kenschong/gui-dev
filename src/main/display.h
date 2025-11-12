#ifndef DISPLAY_H
#define DISPLAY_H

// Forward declaration - tells compiler SpacecraftState exists
struct SpacecraftState;

// Utility functions
float wrapAngle(float angle);
float getThrustLevel(float stick);
float clamp(float value, float min, float max);

// Physics update functions
void updateScenario(SpacecraftState& state, float deltaTime);
void updateSpacecraft(SpacecraftState& state, float deltaTime);

#endif // DISPLAY_H