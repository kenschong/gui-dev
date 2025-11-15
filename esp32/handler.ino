/**
 * Project Mercury Attitude Indicator - ESP32-S3 Version
 * Displays attitude gauges on ST7796 LCD with physics simulation
 */

#include <TFT_eSPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "state.h"
#include "render.h"

// TFT Display
TFT_eSPI tft = TFT_eSPI();
SpacecraftRenderer renderer(&tft);

// Spacecraft state and physics
SpacecraftState state;

// Timing
unsigned long lastUpdate = 0;
unsigned long lastDisplayUpdate = 0;
const unsigned long INPUT_SAMPLE_INTERVAL = 10;   // 100 Hz input sampling
const unsigned long DISPLAY_UPDATE_INTERVAL = 20; // 50 Hz display update
const float PHYSICS_TIMESTEP = 0.01f;             // 100Hz physics

// Joystick/potentiometer pins
const int ROLL_POT_PIN = A0;
const int PITCH_POT_PIN = A1;
const int YAW_POT_PIN = A2;

// Button pins for mode switching
const int BTN_MODE_PIN = 15;

void setup() {
    Serial.begin(115200);
    Serial.println("Project Mercury Attitude Indicator - ESP32-S3");
    
    // Initialize TFT
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    
    // Show splash screen
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(20, 100);
    tft.println("PROJECT MERCURY");
    tft.setCursor(20, 130);
    tft.println("ATTITUDE INDICATOR");
    delay(2000);
    tft.fillScreen(TFT_BLACK);
    
    // Initialize input pins
    pinMode(ROLL_POT_PIN, INPUT);
    pinMode(PITCH_POT_PIN, INPUT);
    pinMode(YAW_POT_PIN, INPUT);
    pinMode(BTN_MODE_PIN, INPUT_PULLUP);
    
    // Initialize spacecraft state
    state.mode = RATE_COMMAND;
    
    Serial.println("Initialization complete");
}

void loop() {
    unsigned long now = millis();
    
    // Input sampling at 100 Hz
    if (now - lastUpdate >= INPUT_SAMPLE_INTERVAL) {
        float deltaTime = (now - lastUpdate) / 1000.0f;
        
        // Handle button inputs
        handleButtons();
        
        // Read joystick
        readControlInputs();
        
        // Update physics
        updateSpacecraft(state, deltaTime);
        
        lastUpdate = now;
    }
    
    // Display update at 50 Hz
    if (now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
        // Render display
        renderer.drawMainDisplay(state);
        
        lastDisplayUpdate = now;
    }
}

void handleButtons() {
    static unsigned long lastModePress = 0;
    static bool lastModeState = HIGH;
    
    // Mode button
    bool modeState = digitalRead(BTN_MODE_PIN);
    if (modeState == LOW && lastModeState == HIGH) {
        if (millis() - lastModePress > 200) {
            cycleMode();
            lastModePress = millis();
        }
    }
    lastModeState = modeState;
}

void cycleMode() {
    switch (state.mode) {
        case MANUAL:
            state.mode = RATE_COMMAND;
            Serial.println("Mode: RATE_COMMAND");
            break;
        case RATE_COMMAND:
            state.mode = FLY_BY_WIRE;
            Serial.println("Mode: FLY_BY_WIRE");
            break;
        case FLY_BY_WIRE:
            state.mode = MANUAL;
            Serial.println("Mode: MANUAL");
            break;
    }
    
    // Reset commands when switching modes
    state.rollCommand = 0;
    state.pitchCommand = 0;
    state.yawCommand = 0;
    state.flyByWireRoll = 0;
    state.flyByWirePitch = 0;
    state.flyByWireYaw = 0;
}

void readControlInputs() {
    // Physical joystick input with ±13° deflection
    // ±13° deflection → ±15°/s rotation rate

    if (state.mode == MANUAL) {
        // Direct rate control
        state.rollRate = map(analogRead(ROLL_POT_PIN), 0, 4095, -15, 15);
        state.pitchRate = map(analogRead(PITCH_POT_PIN), 0, 4095, -15, 15);
        state.yawRate = map(analogRead(YAW_POT_PIN), 0, 4095, -15, 15);
        
        // Apply deadzone: ±2% of 30 range = ±0.6 (round to ±1)
        if (abs(state.rollRate) < 1) state.rollRate = 0;
        if (abs(state.pitchRate) < 1) state.pitchRate = 0;
        if (abs(state.yawRate) < 1) state.yawRate = 0;
        
    } else if (state.mode == RATE_COMMAND) {
        // Commanded rate with realistic thruster physics
        state.rollCommand = map(analogRead(ROLL_POT_PIN), 0, 4095, -15, 15);
        state.pitchCommand = map(analogRead(PITCH_POT_PIN), 0, 4095, -15, 15);
        state.yawCommand = map(analogRead(YAW_POT_PIN), 0, 4095, -15, 15);
        
        // Apply deadzone
        if (abs(state.rollCommand) < 1) state.rollCommand = 0;
        if (abs(state.pitchCommand) < 1) state.pitchCommand = 0;
        if (abs(state.yawCommand) < 1) state.yawCommand = 0;
        
    } else if (state.mode == FLY_BY_WIRE) {
        // On/off thruster control
        state.flyByWireRoll = map(analogRead(ROLL_POT_PIN), 0, 4095, -100, 100);
        state.flyByWirePitch = map(analogRead(PITCH_POT_PIN), 0, 4095, -100, 100);
        state.flyByWireYaw = map(analogRead(YAW_POT_PIN), 0, 4095, -100, 100);
        
        // Apply deadzone
        if (abs(state.flyByWireRoll) < 4) state.flyByWireRoll = 0;
        if (abs(state.flyByWirePitch) < 4) state.flyByWirePitch = 0;
        if (abs(state.flyByWireYaw) < 4) state.flyByWireYaw = 0;
    }
}