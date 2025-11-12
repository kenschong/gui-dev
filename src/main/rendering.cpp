#include "rendering.h"
#include <cmath>

const float PI = 3.14159265359f;
const float DEG_TO_RAD = PI / 180.0f;

void drawAttitudeGauge(ImDrawList* drawList, ImVec2 center, float radius, 
                       float angle, ImU32 color, const char* label, 
                       const char* labels[4]) {
    drawList->AddCircleFilled(center, radius, IM_COL32(26, 26, 26, 255));

    float majorAngles[] = {0, 90, 180, 270};

    for (int i = 0; i < 4; i++) {
        float rad = (majorAngles[i] - 90) * DEG_TO_RAD;
        float x1 = center.x + (radius - 15) * cos(rad);
        float y1 = center.y + (radius - 15) * sin(rad);
        float x2 = center.x + radius * cos(rad);
        float y2 = center.y + radius * sin(rad);
        
        drawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), 
                         IM_COL32(255, 255, 255, 255), 2.0f);
        
        float labelX = center.x + (radius - 30) * cos(rad);
        float labelY = center.y + (radius - 30) * sin(rad);

        ImVec2 textSize = ImGui::CalcTextSize(labels[i]);
        drawList->AddText(ImVec2(labelX - textSize.x/2, labelY - textSize.y/2), 
                         IM_COL32(255, 255, 255, 255), labels[i]);
    }
    
    for (int deg = 0; deg < 360; deg += 30) {
        bool isMajor = (deg == 0 || deg == 90 || deg == 180 || deg == 270);
        
        if (!isMajor) {
            float rad = (deg - 90) * DEG_TO_RAD;
            float x1 = center.x + (radius - 8) * cos(rad);
            float y1 = center.y + (radius - 8) * sin(rad);
            float x2 = center.x + radius * cos(rad);
            float y2 = center.y + radius * sin(rad);

            drawList->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), 
                             IM_COL32(255, 255, 255, 255), 1.0f);
        }
    }
    
    float pointerRad = (angle - 90) * DEG_TO_RAD;
    float pointerLength = radius - 25;
    float endX = center.x + pointerLength * cos(pointerRad);
    float endY = center.y + pointerLength * sin(pointerRad);
    
    drawList->AddLine(center, ImVec2(endX, endY), color, 4.0f);
    drawList->AddCircleFilled(ImVec2(endX, endY), 8.0f, color);
    
    ImVec2 textSize = ImGui::CalcTextSize(label);
    drawList->AddText(ImVec2(center.x - textSize.x/2, center.y + radius + 10), 
                     IM_COL32(255, 255, 255, 255), label);
}

void drawRateIndicator(ImDrawList* drawList, ImVec2 center, float size,
                       float rollRate, float pitchRate, float yawRate) {
    drawList->AddRectFilled(ImVec2(center.x - size/2, center.y - size/2),
                           ImVec2(center.x + size/2, center.y + size/2),
                           IM_COL32(26, 26, 26, 255));
    
    drawList->AddLine(ImVec2(center.x - 70, center.y), 
                     ImVec2(center.x + 70, center.y),
                     IM_COL32(255, 255, 255, 255), 2.0f);
    drawList->AddLine(ImVec2(center.x, center.y - 70),
                     ImVec2(center.x, center.y + 70),
                     IM_COL32(255, 255, 255, 255), 2.0f);
    
    float maxBarLength = 60.0f;
    
    // Roll rate
    float rollBarLength = (rollRate / 100.0f) * maxBarLength;
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
    
    // Pitch rate
    float pitchBarLength = (pitchRate / 100.0f) * maxBarLength;
    drawList->AddRectFilled(ImVec2(center.x + 25, center.y - 2),
                           ImVec2(center.x + 40, center.y + 2),
                           IM_COL32(74, 144, 226, 255));
    if (std::abs(pitchBarLength) > 0) {
        drawList->AddRectFilled(ImVec2(center.x + 30, center.y),
                               ImVec2(center.x + 35, center.y - pitchBarLength),
                               IM_COL32(74, 144, 226, 255));
    }
    
    // Yaw rate
    float yawBarLength = (yawRate / 100.0f) * maxBarLength;
    drawList->AddRectFilled(ImVec2(center.x - 40, center.y - 2),
                           ImVec2(center.x - 25, center.y + 2),
                           IM_COL32(76, 175, 80, 255));
    if (std::abs(yawBarLength) > 0) {
        drawList->AddRectFilled(ImVec2(center.x - 35, center.y),
                               ImVec2(center.x - 30, center.y - yawBarLength),
                               IM_COL32(76, 175, 80, 255));
    }
}