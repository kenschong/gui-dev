#ifndef STATE_H
#define STATE_H

#include "physics.h"

// Control modes
enum ControlMode {
    MANUAL,
    RATE_COMMAND,
    FLY_BY_WIRE
};

// Mission scenarios
enum Scenario {
    NONE,
    RETROFIRE,
    TUMBLE,
    THRUSTER_STUCK,
    ORBITAL_DRIFT
};

// Spacecraft state structure
struct SpacecraftState {
    // Physics engine
    SpacecraftDynamics dynamics;
    
    // Display variables (derived from dynamics)
    float roll              = 0.0f;
    float pitch             = 0.0f;
    float yaw               = 0.0f;
    float rollRate          = 0.0f;
    float pitchRate         = 0.0f;
    float yawRate           = 0.0f;

    // Control inputs
    ControlMode mode        = MANUAL;
    Scenario scenario       = NONE;
    float rollCommand       = 0.0f;
    float pitchCommand      = 0.0f;
    float yawCommand        = 0.0f;
    float flyByWireRoll     = 0.0f;
    float flyByWirePitch    = 0.0f;
    float flyByWireYaw      = 0.0f;

    // Disturbance torques
    float disturbanceRoll   = 0.0f;
    float disturbancePitch  = 0.0f;
    float disturbanceYaw    = 0.0f;
    
    float lastUpdateTime    = 0.0f;
    float scenarioTime      = 0.0f;
    float physicsAccumulator = 0.0f;
};

#endif // STATE_H