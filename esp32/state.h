/**
 * Spacecraft State and Physics - ESP32 Port
 * Based on your original physics.h and state.h
 */

#ifndef STATE_H
#define STATE_H

#include <Arduino.h>
#include <cmath>

// Physical constants
const double PHYSICS_TIMESTEP = 0.01;

// Control modes
enum ControlMode {
    MANUAL,
    RATE_COMMAND,
    FLY_BY_WIRE
};

struct Quaternion {
    double w, x, y, z;
    
    Quaternion() : w(1.0), x(0.0), y(0.0), z(0.0) {}
    Quaternion(double w, double x, double y, double z) : w(w), x(x), y(y), z(z) {}
    
    void normalize() {
        double norm = sqrt(w*w + x*x + y*y + z*z);
        if (norm > 1e-10) {
            w /= norm; x /= norm; y /= norm; z /= norm;
        }
    }
    
    void toRotationMatrix(double R[3][3]) const {
        R[0][0] = 1.0 - 2.0 * (y*y + z*z);
        R[0][1] = 2.0 * (x*y - w*z);
        R[0][2] = 2.0 * (x*z + w*y);
        
        R[1][0] = 2.0 * (x*y + w*z);
        R[1][1] = 1.0 - 2.0 * (x*x + z*z);
        R[1][2] = 2.0 * (y*z - w*x);
        
        R[2][0] = 2.0 * (x*z - w*y);
        R[2][1] = 2.0 * (y*z + w*x);
        R[2][2] = 1.0 - 2.0 * (x*x + y*y);
    }
    
    void toEuler(double& roll, double& pitch, double& yaw) const {
        double R[3][3];
        toRotationMatrix(R);
        
        roll = atan2(R[1][2], R[1][1]) * 180.0 / PI;
        
        double sinPitch = -R[0][2];
        if (sinPitch >= 1.0)
            pitch = 90.0;
        else if (sinPitch <= -1.0)
            pitch = -90.0;
        else
            pitch = asin(sinPitch) * 180.0 / PI;
        
        yaw = atan2(R[0][1], R[0][0]) * 180.0 / PI;
        
        roll = fmod(roll + 360.0, 360.0);
        pitch = fmod(pitch + 360.0, 360.0);
        yaw = fmod(yaw + 360.0, 360.0);
    }
    
    void integrate(double wx, double wy, double wz, double dt) {
        wx *= PI / 180.0;
        wy *= PI / 180.0;
        wz *= PI / 180.0;
        
        double dw = 0.5 * (-x * wx - y * wy - z * wz);
        double dx = 0.5 * ( w * wx + y * wz - z * wy);
        double dy = 0.5 * ( w * wy + z * wx - x * wz);
        double dz = 0.5 * ( w * wz + x * wy - y * wx);
        
        w += dw * dt;
        x += dx * dt;
        y += dy * dt;
        z += dz * dt;
        
        normalize();
    }
};

struct Vec3 {
    double x, y, z;
    
    Vec3() : x(0.0), y(0.0), z(0.0) {}
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {}
    
    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(double s) const { return Vec3(x * s, y * s, z * s); }
    
    double dot(const Vec3& v) const { return x * v.x + y * v.y + z * v.z; }
    Vec3 cross(const Vec3& v) const {
        return Vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }
};

struct SpacecraftDynamics {
    double Ixx = 1000.0;
    double Iyy = 1200.0;
    double Izz = 800.0;
    
    Quaternion orientation;
    Vec3 angularVelocity;  // deg/s
    Vec3 controlTorque;    // N·m
    Vec3 disturbanceTorque;
    
    double thrusterLowTorque = 5.0;
    double thrusterHighTorque = 15.0;
    double thrusterDamping = 0.98;
    
    SpacecraftDynamics() {
        orientation = Quaternion();
        angularVelocity = Vec3(0, 0, 0);
        controlTorque = Vec3(0, 0, 0);
        disturbanceTorque = Vec3(0, 0, 0);
    }
    
    void update(double dt) {
        Vec3 omega(
            angularVelocity.x * PI / 180.0,
            angularVelocity.y * PI / 180.0,
            angularVelocity.z * PI / 180.0
        );
        
        Vec3 Iomega(Ixx * omega.x, Iyy * omega.y, Izz * omega.z);
        Vec3 gyroscopicTorque = omega.cross(Iomega);
        
        Vec3 totalTorque(
            controlTorque.x + disturbanceTorque.x - gyroscopicTorque.x,
            controlTorque.y + disturbanceTorque.y - gyroscopicTorque.y,
            controlTorque.z + disturbanceTorque.z - gyroscopicTorque.z
        );
        
        Vec3 angularAccel(
            totalTorque.x / Ixx,
            totalTorque.y / Iyy,
            totalTorque.z / Izz
        );
        
        angularAccel.x *= 180.0 / PI;
        angularAccel.y *= 180.0 / PI;
        angularAccel.z *= 180.0 / PI;
        
        angularVelocity.x += angularAccel.x * dt;
        angularVelocity.y += angularAccel.y * dt;
        angularVelocity.z += angularAccel.z * dt;
        
        if (abs(controlTorque.x) < 0.1) angularVelocity.x *= thrusterDamping;
        if (abs(controlTorque.y) < 0.1) angularVelocity.y *= thrusterDamping;
        if (abs(controlTorque.z) < 0.1) angularVelocity.z *= thrusterDamping;
        
        orientation.integrate(angularVelocity.x, angularVelocity.y, angularVelocity.z, dt);
    }
    
    void setThrusterCommands(float rollCmd, float pitchCmd, float yawCmd, bool flyByWire) {
        if (flyByWire) {
            controlTorque.x = getThrustTorque(rollCmd);
            controlTorque.y = getThrustTorque(pitchCmd);
            controlTorque.z = getThrustTorque(yawCmd);
        } else {
            controlTorque.x = rollCmd * 0.3;
            controlTorque.y = pitchCmd * 0.3;
            controlTorque.z = yawCmd * 0.3;
        }
    }
    
    double getThrustTorque(float command) {
        double absCmd = abs(command);
        if (absCmd < 25.0) return 0.0;
        
        double sign = (command > 0) ? 1.0 : -1.0;
        if (absCmd < 75.0) return thrusterLowTorque * sign;
        return thrusterHighTorque * sign;
    }
    
    void getEulerAngles(double& roll, double& pitch, double& yaw) const {
        orientation.toEuler(roll, pitch, yaw);
    }
};

struct SpacecraftState {
    SpacecraftDynamics dynamics;
    
    // Display variables (derived from dynamics)
    float roll = 0.0f;
    float pitch = 0.0f;
    float yaw = 0.0f;
    float rollRate = 0.0f;
    float pitchRate = 0.0f;
    float yawRate = 0.0f;
    
    // Control inputs
    ControlMode mode = RATE_COMMAND;
    float rollCommand = 0.0f;
    float pitchCommand = 0.0f;
    float yawCommand = 0.0f;
    float flyByWireRoll = 0.0f;
    float flyByWirePitch = 0.0f;
    float flyByWireYaw = 0.0f;
    
    // Disturbances
    float disturbanceRoll = 0.0f;
    float disturbancePitch = 0.0f;
    float disturbanceYaw = 0.0f;
    
    float lastUpdateTime = 0.0f;
    float physicsAccumulator = 0.0f;
};

float wrapAngle(float angle) {
    while (angle < 0.0f) angle += 360.0f;
    while (angle >= 360.0f) angle -= 360.0f;
    return angle;
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

#endif // STATE_H