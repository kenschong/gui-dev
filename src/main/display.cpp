#include "display.h"
#include "state.h"  // Now we include the full definition
#include <cmath>
#include <cstdlib>

float wrapAngle(float angle) {
    while (angle < 0.0f)    angle += 360.0f;
    while (angle >= 360.0f) angle -= 360.0f;
    return angle;
}

float getThrustLevel(float stick) {
    float absStick = std::abs(stick);
    if (absStick < 25.0f)   return 0;
    if (absStick < 75.0f)   return 1;
    return 2;
}

float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

void updateScenario(SpacecraftState& state, float deltaTime) {
    state.scenarioTime += deltaTime;
    
    if (state.scenario == RETROFIRE) {
        state.disturbanceRoll = std::sin(state.scenarioTime * 0.5f) * 4.0f + 
                               (rand() % 100 - 50) / 50.0f * 1.5f;
        state.disturbancePitch = std::cos(state.scenarioTime * 0.7f) * 3.0f + 
                                (rand() % 100 - 50) / 50.0f * 1.0f;
        state.disturbanceYaw = std::sin(state.scenarioTime * 0.3f) * 2.5f + 
                              (rand() % 100 - 50) / 50.0f * 1.0f;
    } else if (state.scenario == TUMBLE) {
        state.disturbanceRoll = (rand() % 100 - 50) / 50.0f * 15.0f;
        state.disturbancePitch = (rand() % 100 - 50) / 50.0f * 15.0f;
        state.disturbanceYaw = (rand() % 100 - 50) / 50.0f * 15.0f;
    } else if (state.scenario == THRUSTER_STUCK) {
        state.disturbanceRoll = 8.0f;
        state.disturbancePitch = 0.0f;
        state.disturbanceYaw = 0.0f;
    } else if (state.scenario == ORBITAL_DRIFT) {
        state.disturbanceRoll = (rand() % 100 - 50) / 50.0f * 2.0f;
        state.disturbancePitch = (rand() % 100 - 50) / 50.0f * 2.0f;
        state.disturbanceYaw = (rand() % 100 - 50) / 50.0f * 2.0f;
    } else {
        state.disturbanceRoll = 0.0f;
        state.disturbancePitch = 0.0f;
        state.disturbanceYaw = 0.0f;
    }
    
    state.dynamics.disturbanceTorque.x = state.disturbanceRoll;
    state.dynamics.disturbanceTorque.y = state.disturbancePitch;
    state.dynamics.disturbanceTorque.z = state.disturbanceYaw;
}

void updateSpacecraft(SpacecraftState& state, float deltaTime) {
    state.physicsAccumulator += deltaTime;
    
    while (state.physicsAccumulator >= PHYSICS_TIMESTEP) {
        if (state.mode == MANUAL) {
            state.dynamics.angularVelocity.x = state.rollRate;
            state.dynamics.angularVelocity.y = state.pitchRate;
            state.dynamics.angularVelocity.z = state.yawRate;
            
            state.dynamics.orientation.integrate(
                state.dynamics.angularVelocity.x,
                state.dynamics.angularVelocity.y,
                state.dynamics.angularVelocity.z,
                PHYSICS_TIMESTEP
            );
        } else if (state.mode == RATE_COMMAND) {
            state.dynamics.setThrusterCommands(
                state.rollCommand, 
                state.pitchCommand, 
                state.yawCommand, 
                false
            );
            state.dynamics.update(PHYSICS_TIMESTEP);
        } else if (state.mode == FLY_BY_WIRE) {
            state.dynamics.setThrusterCommands(
                state.flyByWireRoll, 
                state.flyByWirePitch, 
                state.flyByWireYaw, 
                true
            );
            state.dynamics.update(PHYSICS_TIMESTEP);
        }
        
        state.physicsAccumulator -= PHYSICS_TIMESTEP;
    }
    
    // Extract display values
    double roll_d, pitch_d, yaw_d;
    state.dynamics.getEulerAngles(roll_d, pitch_d, yaw_d);
    
    state.roll = static_cast<float>(roll_d);
    state.pitch = static_cast<float>(pitch_d);
    state.yaw = static_cast<float>(yaw_d);
    state.rollRate = static_cast<float>(state.dynamics.angularVelocity.x);
    state.pitchRate = static_cast<float>(state.dynamics.angularVelocity.y);
    state.yawRate = static_cast<float>(state.dynamics.angularVelocity.z);
}