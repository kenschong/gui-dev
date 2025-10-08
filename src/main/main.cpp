#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <string>

// Define constants
// Libraries are not used to reduce overhead
const float PI = 3.14159265359f;
const float DEG_TO_RAD = PI / 180.0f;

// Define control modes
enum ControlMode {
    MANUAL,
    RATE_COMMAND,
    FLY_BY_WIRE
};

// Define mission scenarios
enum Scenario {
    NONE,
    RETROFIRE,
    TUMBLE,
    THRUSTER_STUCK,
    ORBITAL_DRIFT
};

// Define default spacecraft state
struct Spacecraft {
    // Define attitude indicator needles
    float roll              = 0.0f;
    float pitch             = 0.0f;
    float yaw               = 0.0f;
    float rollRate          = 0.0f;
    float pitchRate         = 0.0f;
    float yawRate           = 0.0f;

    // Define control inputs
    ControlMode mode        = MANUAL;
    Scenario scenario       = NONE;
    float rollCommand       = 0.0f;
    float pitchCommand      = 0.0f;
    float yawCommand        = 0.0f;
    float flyByWireRoll     = 0.0f;
    float flyByWirePitch    = 0.0f;
    float flyByWireYaw      = 0.0f;

    // Define disturbance torques
    float disturbanceRoll   = 0.0f;
    float disturbancePitch  = 0.0f;
    float disturbanceYaw    = 0.0f;
    
    float lastUpdateTime    = 0.0f;
    float scenarioTime      = 0.0f;
}

// Allows needle on the indicator to rotate continuously
float wrapAngle(float angle) {
    // Wrap angle for all indicator gauge with range
    while (angle < 0.0f)    angle += 360.0f;
    while (angle >= 360.0f) angle -= 360.0f;
    return angle;
}

// Defines which thruster is on (NONE/LOW/HIGH)
float getThrustLevel(float stick) {
    // Retrieve thrust level for fly-by-wire
    // Currently the thrust level is arbitrary
    float absStick = std::abs(stick);
    if (absStick < 25.0f)   return 0;
    if (absStick < 75.0f)   return 1;
    return 2;
}

int main() {
    return 0;
}