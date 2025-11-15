/**
 * Spacecraft Render - TFT Display Version
 * Simplified - Three gauges only, no rate bars
 */

#ifndef RENDER_H
#define RENDER_H

#include <TFT_eSPI.h>
#include "state.h"

// Color definitions (RGB565)
#define COLOR_BACKGROUND  0x2104  // Dark gray
#define COLOR_ROLL        0xFDA0  // Orange
#define COLOR_PITCH       0x4A9E  // Blue
#define COLOR_YAW         0x4CE6  // Green
#define COLOR_WHITE       0xFFFF
#define COLOR_BLACK       0x0000
#define COLOR_GRAY        0x7BEF

class SpacecraftRender {
private:
    TFT_eSPI* tft;
    TFT_eSprite* sprite;
    
    int screenWidth;
    int screenHeight;
    
public:
    SpacecraftRender(TFT_eSPI* display) : tft(display) {
        screenWidth = tft->width();
        screenHeight = tft->height();
        
        // Create sprite for double buffering
        sprite = new TFT_eSprite(tft);
        sprite->createSprite(screenWidth, screenHeight);
        sprite->setTextDatum(MC_DATUM);  // Middle center
    }
    
    ~SpacecraftRender() {
        delete sprite;
    }
    
    void drawMainDisplay(SpacecraftState& state) {
        sprite->fillSprite(COLOR_BACKGROUND);
        
        // For 480x320 landscape display
        if (screenWidth >= 480) {
            drawCompactLayout(state);
        } else {
            drawMinimalLayout(state);
        }
        
        sprite->pushSprite(0, 0);
    }
    
private:
    void drawCompactLayout(SpacecraftState& state) {
        // Title
        sprite->setTextColor(COLOR_WHITE);
        sprite->setTextSize(2);
        sprite->drawString("PROJECT MERCURY", screenWidth / 2, 15);
        
        // Three circular gauges centered vertically
        int gaugeRadius = 60;
        int gaugeY = 160;
        
        drawCircularGauge(80, gaugeY, gaugeRadius, state.roll, COLOR_ROLL, "ROLL");
        drawCircularGauge(240, gaugeY, gaugeRadius, state.pitch, COLOR_PITCH, "PITCH");
        drawCircularGauge(400, gaugeY, gaugeRadius, state.yaw, COLOR_YAW, "YAW");
        
        // Numeric readouts below gauges
        sprite->setTextSize(2);
        char buf[32];
        
        sprintf(buf, "%.0f", state.roll);
        sprite->setTextColor(COLOR_ROLL);
        sprite->drawString(buf, 80, gaugeY + gaugeRadius + 20);
        
        sprintf(buf, "%.0f", state.pitch);
        sprite->setTextColor(COLOR_PITCH);
        sprite->drawString(buf, 240, gaugeY + gaugeRadius + 20);
        
        sprintf(buf, "%.0f", state.yaw);
        sprite->setTextColor(COLOR_YAW);
        sprite->drawString(buf, 400, gaugeY + gaugeRadius + 20);
        
        // Mode display at bottom
        sprite->setTextSize(1);
        sprite->setTextColor(COLOR_WHITE);
        sprite->drawString(getModeString(state.mode), screenWidth / 2, 300);
    }
    
    void drawMinimalLayout(SpacecraftState& state) {
        // For smaller displays - simplified single gauge view
        sprite->setTextSize(1);
        sprite->setTextColor(COLOR_WHITE);
        sprite->drawString("MERCURY", screenWidth / 2, 10);
        
        int centerX = screenWidth / 2;
        int centerY = screenHeight / 2;
        
        drawCircularGauge(centerX, centerY, 80, state.roll, COLOR_ROLL, "ROLL");
        
        // Numeric display
        char buf[64];
        sprite->setTextSize(1);
        sprite->setTextColor(COLOR_WHITE);
        
        sprintf(buf, "P:%.0f Y:%.0f", state.pitch, state.yaw);
        sprite->drawString(buf, centerX, screenHeight - 20);
    }
    
    void drawCircularGauge(int cx, int cy, int radius, float angle, uint16_t color, const char* label) {
        // Outer circle
        sprite->drawCircle(cx, cy, radius, COLOR_WHITE);
        sprite->drawCircle(cx, cy, radius - 1, COLOR_WHITE);
        
        // Cardinal marks (0, 90, 180, 270)
        for (int deg = 0; deg < 360; deg += 90) {
            float rad = (deg - 90) * PI / 180.0f;
            int x1 = cx + (radius - 10) * cos(rad);
            int y1 = cy + (radius - 10) * sin(rad);
            int x2 = cx + radius * cos(rad);
            int y2 = cy + radius * sin(rad);
            sprite->drawLine(x1, y1, x2, y2, COLOR_WHITE);
        }
        
        // Minor marks every 30 degrees
        for (int deg = 0; deg < 360; deg += 30) {
            if (deg % 90 == 0) continue;  // Skip cardinals
            float rad = (deg - 90) * PI / 180.0f;
            int x1 = cx + (radius - 5) * cos(rad);
            int y1 = cy + (radius - 5) * sin(rad);
            int x2 = cx + radius * cos(rad);
            int y2 = cy + radius * sin(rad);
            sprite->drawLine(x1, y1, x2, y2, COLOR_GRAY);
        }
        
        // Pointer
        float pointerRad = (angle - 90) * PI / 180.0f;
        int pointerLength = radius - 15;
        int endX = cx + pointerLength * cos(pointerRad);
        int endY = cy + pointerLength * sin(pointerRad);
        
        sprite->drawLine(cx, cy, endX, endY, color);
        sprite->drawLine(cx + 1, cy, endX + 1, endY, color);
        sprite->fillCircle(endX, endY, 4, color);
        
        // Label
        sprite->setTextColor(COLOR_WHITE);
        sprite->setTextSize(1);
        sprite->drawString(label, cx, cy + radius + 12);
    }
    
    const char* getModeString(ControlMode mode) {
        switch (mode) {
            case MANUAL: return "MANUAL";
            case RATE_COMMAND: return "RATE CMD";
            case FLY_BY_WIRE: return "FLY-BY-WIRE";
            default: return "UNKNOWN";
        }
    }
};

#endif // RENDER_H