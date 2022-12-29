#pragma once

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include "bakkesmod/plugin/bakkesmodplugin.h"

#include <string>
#include <vector>
#include <functional>
#include <memory>

#include "libs/includes/IMGUI/imgui.h"
#include "IMGUI/misc/cpp/imgui_stdlib.h"
#include "IMGUI/imgui_searchablecombo.h"
#include "IMGUI/imgui_rangeslider.h"

#include "fmt/core.h"
#include "fmt/ranges.h"

#include "logging.h"

extern std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

template <typename S, typename... Args>
void LOG(const S &format_str, Args &&...args)
{
	_globalCvarManager->log(fmt::format(format_str, args...));
}