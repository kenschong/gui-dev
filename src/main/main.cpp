#include "design.cpp"
#include "udp_receiver.h"
#include <iostream>

int main() {
    // Initialize GLFW
    if (!glfwInit())
        return -1;
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(1400, 900, 
                                          "Project Mercury Attitude Indicator", 
                                          NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    ImGui::StyleColorsDark();

    SpacecraftState state;

    // Initialize UDP receiver for joystick inputs
    UDPReceiver udpReceiver(8888);
    if (!udpReceiver.start()) {
        std::cerr << "Warning: Failed to start UDP receiver. Continuing without UDP input." << std::endl;
    }
    
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Calculate delta time
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - state.lastUpdateTime;
        state.lastUpdateTime = currentTime;

        // Check for UDP joystick inputs
        JoystickInputPacket joystickInput;
        if (udpReceiver.getLatestInput(joystickInput)) {
            // Apply joystick inputs based on current control mode
            if (state.mode == MANUAL) {
                // In manual mode, map joystick to rate controls directly
                state.rollRate = joystickInput.rollInput;
                state.pitchRate = joystickInput.pitchInput;
                state.yawRate = joystickInput.yawInput;
            } else if (state.mode == RATE_COMMAND) {
                // In rate command mode, map joystick to rate commands (1:1 mapping)
                state.rollCommand = joystickInput.rollInput;
                state.pitchCommand = joystickInput.pitchInput;
                state.yawCommand = joystickInput.yawInput;
            } else if (state.mode == FLY_BY_WIRE) {
                // In fly-by-wire mode, map joystick to stick positions
                state.flyByWireRoll = joystickInput.rollInput;
                state.flyByWirePitch = joystickInput.pitchInput;
                state.flyByWireYaw = joystickInput.yawInput;
            }
        }

        // Update scenario disturbances
        updateScenario(state, deltaTime);

        // Update physics
        updateSpacecraft(state, deltaTime);
        
        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // Create main window
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Mercury Attitude Indicator", nullptr, 
                    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        
        ImGui::SetWindowFontScale(1.2f);
        ImGui::Text("PROJECT MERCURY ATTITUDE INDICATOR");

        // Display UDP status
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 350);
        if (udpReceiver.hasReceivedData()) {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "UDP: CONNECTED");
            ImGui::SameLine();
            if (ImGui::Button("Disconnect UDP")) {
                udpReceiver.reset();
            }
        } else {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "UDP: WAITING (Port %d)", udpReceiver.getPort());
        }

        ImGui::Separator();
        ImGui::Spacing();
        
        // Scenario selection
        ImGui::Text("Mission Scenario:");
        ImGui::SameLine();
        if (ImGui::Button("None")) {
            state.scenario = NONE;
            state.scenarioTime = 0.0f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Retrofire")) {
            state.scenario = RETROFIRE;
            state.scenarioTime = 0.0f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Tumble")) {
            state.scenario = TUMBLE;
            state.scenarioTime = 0.0f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Stuck Thruster")) {
            state.scenario = THRUSTER_STUCK;
            state.scenarioTime = 0.0f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Orbital Drift")) {
            state.scenario = ORBITAL_DRIFT;
            state.scenarioTime = 0.0f;
        }

        // Scenario description
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        if (state.scenario == NONE) {
            ImGui::Text("No disturbances - ideal conditions for testing");
        } else if (state.scenario == RETROFIRE) {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), 
                "RETROFIRE: Varying torques from retrorocket misalignment");
        } else if (state.scenario == TUMBLE) {
            ImGui::TextColored(ImVec4(0.8f, 0.3f, 0.8f, 1.0f), 
                "TUMBLE: High random torques causing rapid rotation");
        } else if (state.scenario == THRUSTER_STUCK) {
            ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f), 
                "STUCK THRUSTER: Constant roll torque - compensate to maintain attitude");
        } else if (state.scenario == ORBITAL_DRIFT) {
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.8f, 1.0f), 
                "ORBITAL DRIFT: Small random disturbances");
        }
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Get draw list
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        
        // Set all gauge positions
        float gaugeRadius = 90.0f;
        ImVec2 rollCenter(250, 200);
        ImVec2 rateCenter(700, 200);
        ImVec2 pitchCenter(1150, 200);
        ImVec2 yawCenter(700, 500);
        
        // Define rate axis labels
        const char* rollLabels[] = {"0", "90", "180", "90"};
        const char* pitchLabels[] = {"0", "90", "180", "90"};
        const char* yawLabels[] = {"0", "90", "180", "270"};
        
        // Draw the attitude indicator 
        drawAttitudeGauge(drawList, rollCenter, gaugeRadius, state.roll, 
                         IM_COL32(255, 165, 0, 255), "ROLL", rollLabels);
        drawAttitudeGauge(drawList, pitchCenter, gaugeRadius, state.pitch,
                         IM_COL32(74, 144, 226, 255), "PITCH", pitchLabels);
        drawAttitudeGauge(drawList, yawCenter, gaugeRadius, state.yaw,
                         IM_COL32(76, 175, 80, 255), "YAW", yawLabels);
        drawRateIndicator(drawList, rateCenter, 180, 
                         state.rollRate, state.pitchRate, state.yawRate);
        
        // Create controls to switch between modes
        ImGui::SetCursorPosY(630);
        ImGui::Text("Control Mode:");
        ImGui::SameLine();

        if (ImGui::Button("MANUAL")) {
            state.mode = MANUAL;
            state.rollCommand = 0; state.pitchCommand = 0; state.yawCommand = 0;
            state.flyByWireRoll = 0; state.flyByWirePitch = 0; state.flyByWireYaw = 0;
            state.rollRate = 0; state.pitchRate = 0; state.yawRate = 0;
        }
        ImGui::SameLine();
        if (ImGui::Button("RATE COMMAND")) {
            state.mode = RATE_COMMAND;
            state.rollCommand = 0; state.pitchCommand = 0; state.yawCommand = 0;
            state.flyByWireRoll = 0; state.flyByWirePitch = 0; state.flyByWireYaw = 0;
        }
        ImGui::SameLine();
        if (ImGui::Button("FLY-BY-WIRE")) {
            state.mode = FLY_BY_WIRE;
            state.rollCommand = 0; state.pitchCommand = 0; state.yawCommand = 0;
            state.flyByWireRoll = 0; state.flyByWirePitch = 0; state.flyByWireYaw = 0;
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Design control inputs based on mode
        if (state.mode == RATE_COMMAND) {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), 
                              "Rate Command Mode - Commands rotation rates");
            ImGui::Spacing();
            
            ImGui::Columns(3, "rateColumns", false);
            
            ImGui::TextColored(ImVec4(1.0f, 0.65f, 0.0f, 1.0f), "Roll Command");
            ImGui::SliderFloat("##rollCmd", &state.rollCommand, -50.0f, 50.0f, "%.0f deg/s");
            ImGui::Text("Attitude: %.0f deg", state.roll);
            ImGui::Text("Rate: %.1f deg/s", state.rollRate);
            
            ImGui::NextColumn();
            
            ImGui::TextColored(ImVec4(0.29f, 0.56f, 0.89f, 1.0f), "Pitch Command");
            ImGui::SliderFloat("##pitchCmd", &state.pitchCommand, -50.0f, 50.0f, "%.0f deg/s");
            ImGui::Text("Attitude: %.0f deg", state.pitch);
            ImGui::Text("Rate: %.1f deg/s", state.pitchRate);
            
            ImGui::NextColumn();
            
            ImGui::TextColored(ImVec4(0.3f, 0.69f, 0.31f, 1.0f), "Yaw Command");
            ImGui::SliderFloat("##yawCmd", &state.yawCommand, -50.0f, 50.0f, "%.0f deg/s");
            ImGui::Text("Attitude: %.0f deg", state.yaw);
            ImGui::Text("Rate: %.1f deg/s", state.yawRate);
            
            ImGui::Columns(1);
            
        } else if (state.mode == FLY_BY_WIRE) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), 
                              "Fly-by-Wire Mode - On/Off thruster control");
            ImGui::Spacing();
            
            ImGui::Columns(3, "fbwColumns", false);
            
            ImGui::TextColored(ImVec4(1.0f, 0.65f, 0.0f, 1.0f), "Roll Stick");
            ImGui::SliderFloat("##rollFBW", &state.flyByWireRoll, -100.0f, 100.0f, "%.0f");
            int rollThrust = getThrustLevel(state.flyByWireRoll);
            ImGui::Text(rollThrust == 0 ? "No Thrust" : rollThrust == 1 ? "LOW Thrust" : "HIGH Thrust");
            ImGui::Text("Attitude: %.0f deg", state.roll);
            ImGui::Text("Rate: %.1f deg/s", state.rollRate);
            
            ImGui::NextColumn();
            
            ImGui::TextColored(ImVec4(0.29f, 0.56f, 0.89f, 1.0f), "Pitch Stick");
            ImGui::SliderFloat("##pitchFBW", &state.flyByWirePitch, -100.0f, 100.0f, "%.0f");
            int pitchThrust = getThrustLevel(state.flyByWirePitch);
            ImGui::Text(pitchThrust == 0 ? "No Thrust" : pitchThrust == 1 ? "LOW Thrust" : "HIGH Thrust");
            ImGui::Text("Attitude: %.0f deg", state.pitch);
            ImGui::Text("Rate: %.1f deg/s", state.pitchRate);
            
            ImGui::NextColumn();
            
            ImGui::TextColored(ImVec4(0.3f, 0.69f, 0.31f, 1.0f), "Yaw Stick");
            ImGui::SliderFloat("##yawFBW", &state.flyByWireYaw, -100.0f, 100.0f, "%.0f");
            int yawThrust = getThrustLevel(state.flyByWireYaw);
            ImGui::Text(yawThrust == 0 ? "No Thrust" : yawThrust == 1 ? "LOW Thrust" : "HIGH Thrust");
            ImGui::Text("Attitude: %.0f deg", state.yaw);
            ImGui::Text("Rate: %.1f deg/s", state.yawRate);
            
            ImGui::Columns(1);
            
        } else {
            ImGui::Text("Manual Mode - Direct control");
            ImGui::Spacing();
            
            ImGui::Columns(3, "manualColumns", false);
            
            ImGui::TextColored(ImVec4(1.0f, 0.65f, 0.0f, 1.0f), "Roll");
            ImGui::SliderFloat("##roll", &state.roll, 0.0f, 360.0f, "%.0f deg");
            ImGui::Text("Roll Rate");
            ImGui::SliderFloat("##rollRate", &state.rollRate, -100.0f, 100.0f, "%.0f");
            
            ImGui::NextColumn();
            
            ImGui::TextColored(ImVec4(0.29f, 0.56f, 0.89f, 1.0f), "Pitch");
            ImGui::SliderFloat("##pitch", &state.pitch, 0.0f, 360.0f, "%.0f deg");
            ImGui::Text("Pitch Rate");
            ImGui::SliderFloat("##pitchRate", &state.pitchRate, -100.0f, 100.0f, "%.0f");
            
            ImGui::NextColumn();
            
            ImGui::TextColored(ImVec4(0.3f, 0.69f, 0.31f, 1.0f), "Yaw");
            ImGui::SliderFloat("##yaw", &state.yaw, 0.0f, 360.0f, "%.0f deg");
            ImGui::Text("Yaw Rate");
            ImGui::SliderFloat("##yawRate", &state.yawRate, -100.0f, 100.0f, "%.0f");
            
            ImGui::Columns(1);
        }
        
        ImGui::End();
        
        // Render and update the window based on user input
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        glfwSwapBuffers(window);
    }
    
    // Properly shutdown the window when closed
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}