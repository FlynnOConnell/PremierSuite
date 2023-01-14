// MIT License
//
// Copyright (c) 2020 Mechatronics and Haptic Interfaces Lab - Rice University
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// Author(s): Evan Pezent (epezent@rice.edu)

#pragma once
#include <IMGUI/imgui.h>
#include <IMGUI/imgui_stdlib.h>

// mahi-gui custom ImGui
namespace ImGui {

bool ModeSelector(int* currentMode, const std::vector<std::string>& modes, bool horizontal);

bool ToggleButton(const char* str_id, bool* v);
bool ButtonColored(const char* label, const ImVec4& color, const ImVec2& size = ImVec2(0,0));
bool ModeSelector(int* currentMode, const std::vector<std::string>& modes, bool horizontal = true);

void StyleColorsMahiDark1();
void StyleColorsMahiDark2();
void StyleColorsMahiDark3();
void StyleColorsMahiDark4();

} // namespace ImGui