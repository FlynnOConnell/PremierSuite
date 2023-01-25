#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "pch.h"
#include <imgui_custom.hpp>
#include "PremierSuite.h"

std::shared_ptr<bool> lowVariance;
std::shared_ptr<bool> medVariance;
std::shared_ptr<bool> highVariance;

namespace ImGui {

bool ModeSelector(int *currentMode, const std::vector<std::string> &modes, bool horizontal) {
    bool changed = false;
    for (std::size_t i = 0; i < modes.size(); ++i) {
        if (ImGui::RadioButton(modes[i].c_str(), (std::size_t)*currentMode == i)) {
            *currentMode = i;
            changed      = true;
        }
        if (horizontal && i + 1 != modes.size())
            ImGui::SameLine();
    }
    return changed;
}

bool ToggleButton(const char* str_id, bool* v)
{
    ImVec4* colors = ImGui::GetStyle().Colors;
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float height = ImGui::GetFrameHeight();
    float width = height * 1.55f;
    float radius = height * 0.50f;

    ImGui::InvisibleButton(str_id, ImVec2(width, height));
    if (ImGui::IsItemClicked()) *v = !*v;
    ImGuiContext& gg = *GImGui;
    float ANIM_SPEED = 0.085f;
    if (gg.LastActiveId == gg.CurrentWindow->GetID(str_id))// && g.LastActiveIdTimer < ANIM_SPEED)
        float t_anim = ImSaturate(gg.LastActiveIdTimer / ANIM_SPEED);
    if (ImGui::IsItemHovered())
        draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*v ? colors[ImGuiCol_ButtonActive] : ImVec4(0.78f, 0.78f, 0.78f, 1.0f)), height * 0.5f);
    else
        draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*v ? colors[ImGuiCol_Button] : ImVec4(0.85f, 0.85f, 0.85f, 1.0f)), height * 0.50f);
    draw_list->AddCircleFilled(ImVec2(p.x + radius + (*v ? 1 : 0) * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
    return *v;

}

bool RadioButton(const char* label, int* v, int v_button, std::shared_ptr<bool> lowVar, std::shared_ptr<bool> medVar, std::shared_ptr<bool> HighVar)
{
    const bool pressed = RadioButton(label, *v == v_button);
    if (pressed)
        *v = v_button;
    if (v_button == 0) { *lowVar = true; *medVar = false; *HighVar = false; }
    if (v_button == 1) { *lowVar = false; *medVar = true; *HighVar = false; }
    if (v_button == 2) { *lowVar = false; *medVar = false; *HighVar = true; }

    return pressed;
}

bool ButtonColored(const char* label, const ImVec4& color, const ImVec2& size) {
    ImVec4 colorHover = ImLerp({ 1, 1, 1, color.w }, color, 0.8f);
    ImVec4 colorPress = ImLerp({ 0, 0, 0, color.w }, color, 0.8f);
    ImGui::PushStyleColor(ImGuiCol_Button, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorHover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorPress);
    bool ret = ImGui::Button(label, size);
    ImGui::PopStyleColor(3);
    return ret;
}

}  // namespace ImGui