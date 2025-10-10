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

// Restrict the rate value to a specific range
float clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// Update the spacecraft physics
void updateSpacecraft(SpacecraftState& state, float deltaTime) {
    // Update rate state for manual mode
    if (state.mode == RATE_COMMAND) {
        // Smooth the rate indicator needles
        state.rollRate += (state.rollCommand - state.rollRate) * 0.3f;
        state.pitchRate += (state.pitchCommand - state.pitchRate) * 0.3f;
        state.yawRate += (state.yawCommand - state.yawRate) * 0.3f;
        
        // Prevent unrealistic spacecraft motion
        state.rollRate = clamp(state.rollRate, -100.0f, 100.0f);
        state.pitchRate = clamp(state.pitchRate, -100.0f, 100.0f);
        state.yawRate = clamp(state.yawRate, -100.0f, 100.0f);
        
        // Update the orientation angles
        state.roll = wrapAngle(state.roll + state.rollRate * deltaTime * 3.0f);
        state.pitch = wrapAngle(state.pitch + state.pitchRate * deltaTime * 3.0f);
        state.yaw = wrapAngle(state.yaw + state.yawRate * deltaTime * 3.0f);
    } else if (state.mode == FLY_BY_WIRE) {
        // Retrieve thrust level from user input
        int rollThrust = getThrustLevel(state.flyByWireRoll);
        int pitchThrust = getThrustLevel(state.flyByWirePitch);
        int yawThrust = getThrustLevel(state.flyByWireYaw);
        
        // Calculate the thrust rate for respective axis
        if (rollThrust > 0) {
            float thrustAmount = (rollThrust == 1) ? 15.0f : 40.0f;
            float direction = (state.flyByWireRoll > 0) ? 1.0f : -1.0f;
            state.rollRate = thrustAmount * direction;
        } else {
            state.rollRate = state.rollRate * 0.95f;
        }
        
        if (pitchThrust > 0) {
            float thrustAmount = (pitchThrust == 1) ? 15.0f : 40.0f;
            float direction = (state.flyByWirePitch > 0) ? 1.0f : -1.0f;
            state.pitchRate = thrustAmount * direction;
        } else {
            state.pitchRate = state.pitchRate * 0.95f;
        }
        
        if (yawThrust > 0) {
            float thrustAmount = (yawThrust == 1) ? 15.0f : 40.0f;
            float direction = (state.flyByWireYaw > 0) ? 1.0f : -1.0f;
            state.yawRate = thrustAmount * direction;
        } else {
            state.yawRate = state.yawRate * 0.95f;
        }
        
        // Prevent unrealistic spacecraft motion
        state.rollRate = clamp(state.rollRate, -100.0f, 100.0f);
        state.pitchRate = clamp(state.pitchRate, -100.0f, 100.0f);
        state.yawRate = clamp(state.yawRate, -100.0f, 100.0f);
        
        // Update the orientation angles
        state.roll = wrapAngle(state.roll + state.rollRate * deltaTime * 3.0f);
        state.pitch = wrapAngle(state.pitch + state.pitchRate * deltaTime * 3.0f);
        state.yaw = wrapAngle(state.yaw + state.yawRate * deltaTime * 3.0f);
    } else {
        // Prevent unrealistic spacecraft motion
        state.rollRate = clamp(state.rollRate, -100.0f, 100.0f);
        state.pitchRate = clamp(state.pitchRate, -100.0f, 100.0f);
        state.yawRate = clamp(state.yawRate, -100.0f, 100.0f);
    }
}

// Draw the individual attitude indicator gauges
void drawAttitudeGauge(ImDrawList* drawList, ImVec2 center, float radius, 
                       float angle, ImU32 color, const char* label, 
                       const char* labels[4]) {
    // Draw a circle for the indicator gauge
    drawList->AddCircleFilled(center, radius, IM_COL32(26, 26, 26, 255));

    // Define major angles for labels
    float majorAngles[] = {0, 90, 180, 270};

    for (int i = 0; i < 4; i++) {
        // Calculate coordinates to draw tick marks
        float rad       = (majorAngles[i] - 90) * DEG_TO_RAD;
        float x1        = center.x + (radius - 15) * cos(rad);
        float y1        = center.y + (radius - 15) * sin(rad);
        float x2        = center.x + radius * cos(rad);
        float y2        = center.y + radius * sin(rad);
        
        // Add tick marks at the major angles
        drawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), 
                         IM_COL32(255, 255, 255, 255), 2.0f);
        
        // Calculate coordinates for desired label location
        float labelX    = center.x + (radius - 30) * cos(rad);
        float labelY    = center.y + (radius - 30) * sin(rad);

        // Add label at the major angles defined before
        ImVec2 textSize = ImGui::CalcTextSize(labels[i]);
        drawList->AddText(ImVec2(labelX - textSize.x/2, labelY - textSize.y/2), 
                         IM_COL32(255, 255, 255, 255), labels[i]);
    }
    
    for (int deg = 0; deg < 360; deg += 30) {
        // Define condition when to add minor tick marks 
        bool isMajor    = (deg == 0 || deg == 90 || deg == 180 || deg == 270);
        
        if (!isMajor) {
            // Calculate coordinates for minor tick marks
            float rad   = (deg - 90) * DEG_TO_RAD;
            float x1    = center.x + (radius - 8) * cos(rad);
            float y1    = center.y + (radius - 8) * sin(rad);
            float x2    = center.x + radius * cos(rad);
            float y2    = center.y + radius * sin(rad);

            // Add minor tick marks to indicator design
            drawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), 
                             IM_COL32(255, 255, 255, 255), 1.0f);
        }
    }
    
    // Calculate the length of the indicator needle
    float pointerRad    = (angle - 90) * DEG_TO_RAD;
    float pointerLength = radius - 25;

    // Calculate coordinates to draw needle
    float endX          = center.x + pointerLength * cos(pointerRad);
    float endY          = center.y + pointerLength * sin(pointerRad);
    
    // Add needle to the indicator gauge design
    drawList->AddLine(center, ImVec2(endX, endY), color, 4.0f);
    drawList->AddCircleFilled(ImVec2(endX, endY), 8.0f, color);
    
    // Add indicator gauge label (ROLL/PITCH/YAW)
    ImVec2 textSize = ImGui::CalcTextSize(label);
    drawList->AddText(ImVec2(center.x - textSize.x/2, center.y + radius + 10), 
                     IM_COL32(255, 255, 255, 255), label);
}

void drawRateIndicator(ImDrawList* drawList, ImVec2 center, float size,
                       float rollRate, float pitchRate, float yawRate) {
    // Draw the rate indicator background
    drawList->AddRectFilled(ImVec2(center.x - size/2, center.y - size/2),
                           ImVec2(center.x + size/2, center.y + size/2),
                           IM_COL32(26, 26, 26, 255));
    
    // Draw the reference crosshairs
    drawList->AddLine(ImVec2(center.x - 70, center.y), 
                     ImVec2(center.x + 70, center.y),
                     IM_COL32(255, 255, 255, 255), 2.0f);
    drawList->AddLine(ImVec2(center.x, center.y - 70),
                     ImVec2(center.x, center.y + 70),
                     IM_COL32(255, 255, 255, 255), 2.0f);
    
    // Define rate bar
    float maxBarLength = 60.0f;
    
    // Define roll rate for the reference crosshair
    float rollBarLength = (rollRate / 100.0f) * maxBarLength;

    // Update the reference crosshair accordingly
    drawList->AddRectFilled(ImVec2(center.x - 2, center.y - 40),
                           ImVec2(center.x + 2, center.y - 25),
                           IM_COL32(255, 165, 0, 255));
    drawList->AddRectFilled(ImVec2(center.x - 2, center.y + 25),
                           ImVec2(center.x + 2, center.y + 40),
                           IM_COL32(255, 165, 0, 255));
    if (std::abs(rollBarLength) > 0) {
        drawList->AddRectFilled(ImVec2(center.x, center.y - 35),
                               ImVec2(center.x + rollBarLength, center.y - 30),
                               IM_COL32(255, 165, 0, 255));
    }
    
    // Define pitch rate for the reference crosshair
    float pitchBarLength = (pitchRate / 100.0f) * maxBarLength;

    // Update the reference crosshair accordingly
    drawList->AddRectFilled(ImVec2(center.x + 25, center.y - 2),
                           ImVec2(center.x + 40, center.y + 2),
                           IM_COL32(74, 144, 226, 255));
    if (std::abs(pitchBarLength) > 0) {
        drawList->AddRectFilled(ImVec2(center.x + 30, center.y),
                               ImVec2(center.x + 35, center.y - pitchBarLength),
                               IM_COL32(74, 144, 226, 255));
    }
    
    // Define yaw rate for the reference crosshair
    float yawBarLength = (yawRate / 100.0f) * maxBarLength;

    // Update the reference crosshair accordingly
    drawList->AddRectFilled(ImVec2(center.x - 40, center.y - 2),
                           ImVec2(center.x - 25, center.y + 2),
                           IM_COL32(76, 175, 80, 255));
    if (std::abs(yawBarLength) > 0) {
        drawList->AddRectFilled(ImVec2(center.x - 35, center.y),
                               ImVec2(center.x - 30, center.y - yawBarLength),
                               IM_COL32(76, 175, 80, 255));
    }
}