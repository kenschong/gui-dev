#ifndef RENDERING_H
#define RENDERING_H

#include <imgui.h>

// Drawing functions
void drawAttitudeGauge(ImDrawList* drawList, ImVec2 center, float radius, 
                       float angle, ImU32 color, const char* label, 
                       const char* labels[4]);

void drawRateIndicator(ImDrawList* drawList, ImVec2 center, float size,
                       float rollRate, float pitchRate, float yawRate);

#endif // RENDERING_H