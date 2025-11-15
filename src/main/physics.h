#ifndef PHYSICS_H
#define PHYSICS_H

#include <cmath>

/**
 * Spacecraft Physics Module
 * Uses independent axis extraction to avoid gimbal lock in display
 */

// Physical constants
const double PHYSICS_TIMESTEP = 0.01;  // 100 Hz physics update rate

// Quaternion structure for orientation representation
struct Quaternion {
    double w, x, y, z;
    
    Quaternion() : w(1.0), x(0.0), y(0.0), z(0.0) {}
    Quaternion(double w, double x, double y, double z) : w(w), x(x), y(y), z(z) {}
    
    // Normalize quaternion
    void normalize() {
        double norm = std::sqrt(w*w + x*x + y*y + z*z);
        if (norm > 1e-10) {
            w /= norm; x /= norm; y /= norm; z /= norm;
        }
    }
    
    // Convert quaternion to rotation matrix
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
    
    // Extract display angles without gimbal lock
    // Projects body axes onto reference frame to get independent angles
    void toEuler(double& roll, double& pitch, double& yaw) const {
        // Get rotation matrix
        double R[3][3];
        toRotationMatrix(R);
        
        // Extract roll: rotation of body Y-axis around body X-axis
        // Project body Y onto the ground plane and measure angle from horizon
        roll = std::atan2(R[1][2], R[1][1]) * 180.0 / M_PI;
        
        // Extract pitch: angle of body X-axis from horizon
        // Use asin for pitch (yes, limited to [-90,90], but that's physically correct)
        double sinPitch = -R[0][2];
        if (sinPitch >= 1.0)
            pitch = 90.0;
        else if (sinPitch <= -1.0)
            pitch = -90.0;
        else
            pitch = std::asin(sinPitch) * 180.0 / M_PI;
        
        // Extract yaw: rotation of body X-axis projection in ground plane
        yaw = std::atan2(R[0][1], R[0][0]) * 180.0 / M_PI;
        
        // Wrap to [0, 360)
        roll = fmod(roll + 360.0, 360.0);
        pitch = fmod(pitch + 360.0, 360.0);
        yaw = fmod(yaw + 360.0, 360.0);
    }
    
    // Integrate angular velocity into quaternion
    void integrate(double wx, double wy, double wz, double dt) {
        // Convert angular velocity from deg/s to rad/s
        wx *= M_PI / 180.0;
        wy *= M_PI / 180.0;
        wz *= M_PI / 180.0;
        
        // Quaternion derivative
        double dw = 0.5 * (-x * wx - y * wy - z * wz);
        double dx = 0.5 * ( w * wx + y * wz - z * wy);
        double dy = 0.5 * ( w * wy + z * wx - x * wz);
        double dz = 0.5 * ( w * wz + x * wy - y * wx);
        
        // Integrate
        w += dw * dt;
        x += dx * dt;
        y += dy * dt;
        z += dz * dt;
        
        normalize();
    }
};

// 3D vector structure
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

/**
 * SpacecraftDynamics - Rigid body rotation physics
 */
struct SpacecraftDynamics {
    // Moment of inertia (kg·m²)
    double Ixx = 1000.0;
    double Iyy = 1200.0;
    double Izz = 800.0;
    
    // State variables
    Quaternion orientation;
    Vec3 angularVelocity;  // deg/s
    
    // Control and disturbance torques (N·m)
    Vec3 controlTorque;
    Vec3 disturbanceTorque;
    
    // Thruster parameters (N·m)
    double thrusterLowTorque = 5.0;
    double thrusterHighTorque = 15.0;
    double thrusterDamping = 0.98;
    
    SpacecraftDynamics() {
        orientation = Quaternion();
        angularVelocity = Vec3(0, 0, 0);
        controlTorque = Vec3(0, 0, 0);
        disturbanceTorque = Vec3(0, 0, 0);
    }
    
    /**
     * Update physics using Euler's equations of motion
     */
    void update(double dt) {
        // Convert angular velocity to rad/s
        Vec3 omega(
            angularVelocity.x * M_PI / 180.0,
            angularVelocity.y * M_PI / 180.0,
            angularVelocity.z * M_PI / 180.0
        );
        
        // Calculate gyroscopic torque: ω × (I * ω)
        Vec3 Iomega(Ixx * omega.x, Iyy * omega.y, Izz * omega.z);
        Vec3 gyroscopicTorque = omega.cross(Iomega);
        
        // Total torque
        Vec3 totalTorque(
            controlTorque.x + disturbanceTorque.x - gyroscopicTorque.x,
            controlTorque.y + disturbanceTorque.y - gyroscopicTorque.y,
            controlTorque.z + disturbanceTorque.z - gyroscopicTorque.z
        );
        
        // Angular acceleration: α = I^(-1) * T
        Vec3 angularAccel(
            totalTorque.x / Ixx,
            totalTorque.y / Iyy,
            totalTorque.z / Izz
        );
        
        // Convert back to deg/s²
        angularAccel.x *= 180.0 / M_PI;
        angularAccel.y *= 180.0 / M_PI;
        angularAccel.z *= 180.0 / M_PI;
        
        // Integrate angular velocity
        angularVelocity.x += angularAccel.x * dt;
        angularVelocity.y += angularAccel.y * dt;
        angularVelocity.z += angularAccel.z * dt;
        
        // Apply damping when no control torque
        if (std::abs(controlTorque.x) < 0.1) angularVelocity.x *= thrusterDamping;
        if (std::abs(controlTorque.y) < 0.1) angularVelocity.y *= thrusterDamping;
        if (std::abs(controlTorque.z) < 0.1) angularVelocity.z *= thrusterDamping;
        
        // Integrate orientation
        orientation.integrate(angularVelocity.x, angularVelocity.y, angularVelocity.z, dt);
    }
    
    /**
     * Set control torques based on thruster commands
     */
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
    
    /**
     * Get torque from thruster command
     */
    double getThrustTorque(float command) {
        double absCmd = std::abs(command);
        if (absCmd < 25.0) return 0.0;
        
        double sign = (command > 0) ? 1.0 : -1.0;
        if (absCmd < 75.0) return thrusterLowTorque * sign;
        return thrusterHighTorque * sign;
    }
    
    /**
     * Get current Euler angles
     */
    void getEulerAngles(double& roll, double& pitch, double& yaw) const {
        orientation.toEuler(roll, pitch, yaw);
    }
    
    /**
     * Reset to initial state
     */
    void reset() {
        orientation = Quaternion();
        angularVelocity = Vec3(0, 0, 0);
        controlTorque = Vec3(0, 0, 0);
        disturbanceTorque = Vec3(0, 0, 0);
    }
};

#endif // PHYSICS_H