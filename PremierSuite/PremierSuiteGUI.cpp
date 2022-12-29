#include "pch.h"
#include "PremierSuite.h"
#include "IMGUI/imgui.h"
#include "IMGUI/imgui_internal.h"


#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

// System includes
#include <ctype.h>          // toupper
#include <limits.h>         // INT_MIN, INT_MAX
#include <math.h>           // sqrtf, powf, cosf, sinf, floorf, ceilf
#include <stdio.h>          // vsnprintf, sscanf, printf
#include <stdlib.h>         // NULL, malloc, free, atoi
#if defined(_MSC_VER) && _MSC_VER <= 1500 // MSVC 2008 or earlier
#include <stddef.h>         // intptr_t
#else
#include <stdint.h>         // intptr_t
#endif

// Disable annoying Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (disable: 4127)     // condition expression is constant
#pragma warning (disable: 4996)     // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#pragma warning (disable: 26451)    // [Static Analyzer] Arithmetic overflow : Using operator 'xxx' on a 4 byte value and then casting the result to an 8 byte value. Cast the value to the wider type before calling operator 'xxx' to avoid overflow(io.2).
#endif

//-----------------------------------------------------------------------------
// Clang/GCC warnings with -Weverything
//-----------------------------------------------------------------------------

#if defined(__clang__)
#if __has_warning("-Wunknown-warning-option")
#pragma clang diagnostic ignored "-Wunknown-warning-option"         // not all warnings are known by all Clang versions and they tend to be rename-happy.. so ignoring warnings triggers new warnings on some configuration. Great!
#endif
#pragma clang diagnostic ignored "-Wunknown-pragmas"                // warning: unknown warning group 'xxx'
#pragma clang diagnostic ignored "-Wold-style-cast"                 // warning: use of old-style cast                           // yes, they are more terse.
#pragma clang diagnostic ignored "-Wdeprecated-declarations"        // warning: 'xx' is deprecated: The POSIX name for this..   // for strdup used in demo code (so user can copy & paste the code)
#pragma clang diagnostic ignored "-Wint-to-void-pointer-cast"       // warning: cast to 'void *' from smaller integer type
#pragma clang diagnostic ignored "-Wformat-security"                // warning: format string is not a string literal
#pragma clang diagnostic ignored "-Wexit-time-destructors"          // warning: declaration requires an exit-time destructor    // exit-time destruction order is undefined. if MemFree() leads to users code that has been disabled before exit it might cause problems. ImGui coding style welcomes static/globals.
#pragma clang diagnostic ignored "-Wunused-macros"                  // warning: macro is not used                               // we define snprintf/vsnprintf on Windows so they are available, but not always used.
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"  // warning: zero as null pointer constant                   // some standard header variations use #define NULL 0
#pragma clang diagnostic ignored "-Wdouble-promotion"               // warning: implicit conversion from 'float' to 'double' when passing argument to function  // using printf() is a misery with this as C++ va_arg ellipsis changes float to double.
#pragma clang diagnostic ignored "-Wreserved-id-macro"              // warning: macro name is a reserved identifier
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"  // warning: implicit conversion from 'xxx' to 'float' may lose precision
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wpragmas"                  // warning: unknown option after '#pragma GCC diagnostic' kind
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"      // warning: cast to pointer from integer of different size
#pragma GCC diagnostic ignored "-Wformat-security"          // warning: format string is not a string literal (potentially insecure)
#pragma GCC diagnostic ignored "-Wdouble-promotion"         // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"               // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#pragma GCC diagnostic ignored "-Wmisleading-indentation"   // [__GNUC__ >= 6] warning: this 'if' clause does not guard this statement      // GCC 6.0+ only. See #883 on GitHub.
#endif

#ifdef _WIN32
#define IM_NEWLINE  "\r\n"
#else
#define IM_NEWLINE  "\n"
#endif

//-----------------------------------------------------------------------------
// Helpers / Workers
//-----------------------------------------------------------------------------

#if defined(_MSC_VER) && !defined(snprintf)
#define snprintf    _snprintf
#endif
#if defined(_MSC_VER) && !defined(vsnprintf)
#define vsnprintf   _vsnprintf
#endif

// Format specifiers, printing 64-bit hasn't been decently standardized...
// In a real application you should be using PRId64 and PRIu64 from <inttypes.h> (non-windows) and on Windows define them yourself.
#ifdef _MSC_VER
#define IM_PRId64   "I64d"
#define IM_PRIu64   "I64u"
#else
#define IM_PRId64   "lld"
#define IM_PRIu64   "llu"
#endif

// Helpers macros
#define IM_MIN(A, B)            (((A) < (B)) ? (A) : (B))
#define IM_MAX(A, B)            (((A) >= (B)) ? (A) : (B))
#define IM_CLAMP(V, MN, MX)     ((V) < (MN) ? (MN) : (V) > (MX) ? (MX) : (V))

// Enforce cdecl calling convention for functions called by the standard library, in case compilation settings changed the default to e.g. __vectorcall
#ifndef IMGUI_CDECL
#ifdef _MSC_VER
#define IMGUI_CDECL __cdecl
#else
#define IMGUI_CDECL
#endif
#endif

extern std::filesystem::path PremierSuiteDataFolder;

extern bool ImGuiSaveStyle(const char* filename, const ImGuiStyle& style);
extern bool ImGuiLoadStyle(const char* filename, ImGuiStyle& style);

static void ShowExampleAppConsole(bool* p_open);
static void ShowExampleAppLog(bool* p_open);

extern void EmbraceTheDarknessTheme();
extern void MinimalMatteTheme();
extern void DarkRedTheme();

static void HelpMarker(const char* desc)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

#define IM_COL32_ERROR_RED (ImColor)IM_COL32(204,0,0,255)
typedef void (*ImGuiDemoMarkerCallback)(const char* file, int line, const char* section, void* user_data);
extern ImGuiDemoMarkerCallback      GImGuiDemoMarkerCallback;
extern void* GImGuiDemoMarkerCallbackUserData;
ImGuiDemoMarkerCallback             GImGuiDemoMarkerCallback = NULL;
void* GImGuiDemoMarkerCallbackUserData = NULL;
#define IMGUI_DEMO_MARKER(section)  do { if (GImGuiDemoMarkerCallback != NULL) GImGuiDemoMarkerCallback(__FILE__, __LINE__, section, GImGuiDemoMarkerCallbackUserData); } while (0)

/// <summary>ImGui widgets to render.</summary>
void PremierSuite::Render()
{	
	/*cvarManager->executeCommand("exec config.cfg");*/
	IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context.");

	ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(menuTitle.c_str(), &isWindowOpen, ImGuiWindowFlags_MenuBar))
	{
		if (ImGui::BeginMenuBar())
		{
			renderMenu();
			ImGui::EndMenuBar();
		}
		if (ImGui::BeginTabBar("#TabBar", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_NoTooltip)) {
			ImGui::Indent(5);
			ImGui::Spacing();
			renderSettingsTab();
			renderKeybindsTab();
			ImGui::EndTabBar();
		}
		
		if (!isWindowOpen) {
			cvarManager->executeCommand("togglemenu " + GetMenuName());
		}
	}
	ImGui::End();
}

//-----------------------------------------------------------------------------
// Render ImGui Widgets
//-----------------------------------------------------------------------------

/// <summary> Renders main menu. </summary>
void PremierSuite::renderMenu()
{
	static bool show_app_console = false;
	static bool show_app_log = false;

	if (show_app_console)             ShowExampleAppConsole(&show_app_console);
	if (show_app_log)                 ShowExampleAppLog(&show_app_log);

	// Dear ImGui Apps (accessible from the "Tools" menu)
	static bool show_app_metrics = false;
	static bool show_app_style_editor = false;
	static bool show_app_about = false;

	ImGuiStyle& style = ImGui::GetStyle();

	if (show_app_style_editor) { ImGui::Begin("PremierSuite Style Editor", &show_app_style_editor); OpenStyleEditorWindow(&style); ImGui::End(); }
	if (show_app_about) { renderAboutWindow(&show_app_about); }
	if (show_app_metrics) { ImGui::ShowMetricsWindow(&show_app_metrics); }

	// Various window flags. Typically you would just use the default!
	static bool no_titlebar = false;
	static bool no_scrollbar = false;
	static bool no_menu = false;
	static bool no_move = false;
	static bool no_resize = false;
	static bool no_collapse = false;
	static bool no_close = false;
	static bool no_nav = false;
	static bool no_background = false;
	static bool no_bring_to_front = false;

	ImGuiWindowFlags window_flags = 0;
	if (no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
	if (no_scrollbar)       window_flags |= ImGuiWindowFlags_NoScrollbar;
	if (!no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
	if (no_move)            window_flags |= ImGuiWindowFlags_NoMove;
	if (no_resize)          window_flags |= ImGuiWindowFlags_NoResize;
	if (no_collapse)        window_flags |= ImGuiWindowFlags_NoCollapse;
	if (no_nav)             window_flags |= ImGuiWindowFlags_NoNav;
	if (no_background)      window_flags |= ImGuiWindowFlags_NoBackground;
	if (no_bring_to_front)  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

	if (ImGui::BeginMenu("Tools"))
	{
		HelpMarker("Tools for developers.");
		ImGui::MenuItem("Style Editor", NULL, &show_app_style_editor);
		ImGui::MenuItem("Console", NULL, &show_app_console);
		ImGui::MenuItem("Log", NULL, &show_app_log);
		ImGui::EndMenu();
	}
	//if (ImGui::MenuItem("MenuItem")) {} // You can also use MenuItem() inside a menu bar!
	if (ImGui::BeginMenu("Metrics"))
	{
		const bool has_debug_tools = true;

		ImGui::MenuItem("Metrics/Debugger", NULL, &show_app_metrics, has_debug_tools);
		ImGui::MenuItem("About PremierSuite", NULL, &show_app_about);
		ImGui::EndMenu();
	}
	ImGui::Indent(5);
	ImGui::Spacing();
}

/// <summary> Renders keybinds tab for changing GUI keybinds. </summary>
void PremierSuite::renderKeybindsTab()
{
	if (ImGui::BeginTabItem("Keybinds"))
	{
		if (ImGui::BeginChild("Keybinds"))
		{
			ImGui::Dummy(ImVec2(0.0f, 5.0f));

			ImGui::Text(
				"Easily Change keybinds to whatever button you'd like.\n"
			);
			ImGui::Separator();
			ImGui::Indent(5);
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			static char keybindInput[128] = "";

			auto currentKb = cvarManager->getCvar("plugin_keybind");
			if (!currentKb) { return; }
			std::string currentKbString = currentKb.getStringValue();
			
			ImGui::PushItemWidth(15.0f * ImGui::GetFontSize());
			ImGui::InputTextWithHint("", "Type out your desired keybind", keybindInput, IM_ARRAYSIZE(keybindInput), ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_CharsUppercase);
			ImGui::PopItemWidth();

			if (ImGui::Button("Change GUI Keybind"))
			{
				cvarManager->removeBind(currentKbString);
				cvarManager->setBind(keybindInput, "togglemenu " + GetMenuName());
				currentKb.setValue(keybindInput);
				cvarManager->executeCommand("writeconfig", true);
			}
			ImGui::Separator();
			ImGui::Indent(5);
			ImGui::Spacing();

			ImGui::Text("Current GUI keybind: % s\n", currentKbString);
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("\n"
					"This may not be updated if loading plugin for the first time..\n\n");
			ImGui::EndChild();
		}
		ImGui::EndTabItem();
	}
}

/// <summary> Renders configurable style editor. </summary>
void PremierSuite::OpenStyleEditorWindow(ImGuiStyle* ref)
{

	IMGUI_DEMO_MARKER("Style Editor");
	static ImGuiStyle style;
	static ImGuiStyle ref_saved_style;
	if (ref == NULL)
		ImGuiStyle& style = ImGui::GetStyle();
	if (ref)
		ImGuiStyle style = *ref;

	if (renderStyleCombo("Colors##Selector"))
		ref_saved_style = style;

	// Simplified Settings (expose floating-pointer border sizes as boolean representing 0.0f or 1.0f)
	if (ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f"))
		style.GrabRounding = style.FrameRounding; // Make GrabRounding always the same value as FrameRounding
	{ bool border = (style.WindowBorderSize > 0.0f); if (ImGui::Checkbox("WindowBorder", &border)) { style.WindowBorderSize = border ? 1.0f : 0.0f; } }
	ImGui::SameLine();
	{ bool border = (style.FrameBorderSize > 0.0f);  if (ImGui::Checkbox("FrameBorder", &border)) { style.FrameBorderSize = border ? 1.0f : 0.0f; } }
	ImGui::SameLine();
	{ bool border = (style.PopupBorderSize > 0.0f);  if (ImGui::Checkbox("PopupBorder", &border)) { style.PopupBorderSize = border ? 1.0f : 0.0f; } }

	// Save/Revert button
	if (ImGui::Button("Save Ref"))
		*ref = ref_saved_style = style;
	ImGui::SameLine();
	if (ImGui::Button("Save To File"))
		*ref = ref_saved_style = style;
	ImGui::SameLine();
	if (ImGui::Button("Revert Ref"))
		style = *ref;
	ImGui::SameLine();
	HelpMarker(
		"Save/Revert in local non-persistent storage. Default Colors definition are not affected. "
		"Use \"Export\" below to save them somewhere.");

	ImGui::Separator();

	if (ImGui::BeginTabBar("##tabs"))
	{
		if (ImGui::BeginTabItem("Sizes"))
		{
			ImGui::Text("Main");
			ImGui::SliderFloat2("WindowPadding", (float*)&style.WindowPadding, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("FramePadding", (float*)&style.FramePadding, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("ItemSpacing", (float*)&style.ItemSpacing, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("ItemInnerSpacing", (float*)&style.ItemInnerSpacing, 0.0f, 20.0f, "%.0f");
			ImGui::SliderFloat2("TouchExtraPadding", (float*)&style.TouchExtraPadding, 0.0f, 10.0f, "%.0f");
			ImGui::SliderFloat("IndentSpacing", &style.IndentSpacing, 0.0f, 30.0f, "%.0f");
			ImGui::SliderFloat("ScrollbarSize", &style.ScrollbarSize, 1.0f, 20.0f, "%.0f");
			ImGui::SliderFloat("GrabMinSize", &style.GrabMinSize, 1.0f, 20.0f, "%.0f");
			ImGui::Text("Borders");
			ImGui::SliderFloat("WindowBorderSize", &style.WindowBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("ChildBorderSize", &style.ChildBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("PopupBorderSize", &style.PopupBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("FrameBorderSize", &style.FrameBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::SliderFloat("TabBorderSize", &style.TabBorderSize, 0.0f, 1.0f, "%.0f");
			ImGui::Text("Rounding");
			ImGui::SliderFloat("WindowRounding", &style.WindowRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("ChildRounding", &style.ChildRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("FrameRounding", &style.FrameRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("PopupRounding", &style.PopupRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("ScrollbarRounding", &style.ScrollbarRounding, 0.0f, 12.0f, "%.0f");
			ImGui::SliderFloat("GrabRounding", &style.GrabRounding, 0.0f, 12.0f, "%.0f");
			ImGui::Text("Alignment");
			ImGui::SliderFloat2("WindowTitleAlign", (float*)&style.WindowTitleAlign, 0.0f, 1.0f, "%.2f");
			int window_menu_button_position = style.WindowMenuButtonPosition + 1;
			if (ImGui::Combo("WindowMenuButtonPosition", (int*)&window_menu_button_position, "None\0Left\0Right\0"))
				style.WindowMenuButtonPosition = window_menu_button_position - 1;
			ImGui::Combo("ColorButtonPosition", (int*)&style.ColorButtonPosition, "Left\0Right\0");
			ImGui::SliderFloat2("ButtonTextAlign", (float*)&style.ButtonTextAlign, 0.0f, 1.0f, "%.2f");
			ImGui::SameLine(); HelpMarker("Alignment applies when a button is larger than its text content.");
			ImGui::SliderFloat2("SelectableTextAlign", (float*)&style.SelectableTextAlign, 0.0f, 1.0f, "%.2f");
			ImGui::SameLine(); HelpMarker("Alignment applies when a selectable is larger than its text content.");
			ImGui::Text("Safe Area Padding");
			ImGui::SameLine(); HelpMarker("Adjust if you cannot see the edges of your screen (e.g. on a TV where scaling has not been configured).");
			ImGui::SliderFloat2("DisplaySafeAreaPadding", (float*)&style.DisplaySafeAreaPadding, 0.0f, 30.0f, "%.0f");
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Colors"))
		{
			static int output_dest = 0;
			static bool output_only_modified = true;
			if (ImGui::Button("Export"))
			{
				if (output_dest == 0)
					ImGui::LogToClipboard();
				else
					ImGui::LogToTTY();
				ImGui::LogText("ImVec4* colors = ImGui::GetStyle().Colors;" IM_NEWLINE);
				for (int i = 0; i < ImGuiCol_COUNT; i++)
				{
					const ImVec4& col = style.Colors[i];
					const char* name = ImGui::GetStyleColorName(i);
					if (!output_only_modified || memcmp(&col, &ref->Colors[i], sizeof(ImVec4)) != 0)
						ImGui::LogText("colors[ImGuiCol_%s]%*s= ImVec4(%.2ff, %.2ff, %.2ff, %.2ff);" IM_NEWLINE,
							name, 23 - (int)strlen(name), "", col.x, col.y, col.z, col.w);
				}
				ImGui::LogFinish();
			}
			ImGui::SameLine(); ImGui::SetNextItemWidth(120); ImGui::Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0");
			ImGui::SameLine(); ImGui::Checkbox("Only Modified Colors", &output_only_modified);

			static ImGuiTextFilter filter;
			filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

			static ImGuiColorEditFlags alpha_flags = 0;
			if (ImGui::RadioButton("Opaque", alpha_flags == ImGuiColorEditFlags_None)) { alpha_flags = ImGuiColorEditFlags_None; } ImGui::SameLine();
			if (ImGui::RadioButton("Alpha", alpha_flags == ImGuiColorEditFlags_AlphaPreview)) { alpha_flags = ImGuiColorEditFlags_AlphaPreview; } ImGui::SameLine();
			if (ImGui::RadioButton("Both", alpha_flags == ImGuiColorEditFlags_AlphaPreviewHalf)) { alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf; } ImGui::SameLine();
			HelpMarker(
				"In the color list:\n"
				"Left-click on color square to open color picker,\n"
				"Right-click to open edit options menu.");

			ImGui::BeginChild("##colors", ImVec2(0, ImGui::GetFontSize() * 20.0f), true, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::PushItemWidth(-160);
			for (int i = 0; i < ImGuiCol_COUNT; i++)
			{
				const char* name = ImGui::GetStyleColorName(i);
				if (!filter.PassFilter(name))
					continue;
				ImGui::PushID(i);
				ImGui::ColorEdit4("##color", (float*)&style.Colors[i], ImGuiColorEditFlags_AlphaBar | alpha_flags);
				if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(ImVec4)) != 0)
				{
					ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Save")) { ref->Colors[i] = style.Colors[i]; }
					ImGui::SameLine(0.0f, style.ItemInnerSpacing.x); if (ImGui::Button("Revert")) { style.Colors[i] = ref->Colors[i]; }
				}
				ImGui::SameLine(0.0f, style.ItemInnerSpacing.x);
				ImGui::TextUnformatted(name);
				ImGui::PopID();
			}
			ImGui::PopItemWidth();
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	
}

/// <summary> Renders main GUI settings. </summary>
void PremierSuite::renderSettingsTab()
{
	if (ImGui::BeginTabItem("Settings"))
	{
		ImGui::Spacing();
		ImGui::Text(
			"PremierSuite:\n\n"
			"Instant Access to Auto-Queue, Freeplay, and Custom Training."
		);
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Indent(5);
		ImGui::Spacing();

		// ENABLE PLUGIN
		static auto pluginCvar = cvarManager->getCvar("plugin_enabled");
		auto pluginEnabled = pluginCvar.getBoolValue();
		
		if (ImGui::Checkbox("Enable Plugin", &pluginEnabled)) {
			pluginCvar.setValue(pluginEnabled);
			cvarManager->executeCommand("writeconfig", false);
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Enable/Disable the plugin and all of its functionality.\n"
				"You can also bind this to a key in the Keybinds tab!.\n\n");

		ImGui::BeginChild("Plugin Options", ImVec2(0, 0));
		ImGui::Spacing();
		ImGui::Spacing();

		// DISABLE PRIVATE
		static auto disablePrivateCvar = cvarManager->getCvar(disablePrivateCvarName);
		auto disablePrivate = disablePrivateCvar.getBoolValue();
		if (ImGui::Checkbox("Disable for Private", &disablePrivate)) {
			disablePrivateCvar.setValue(disablePrivate);
			cvarManager->executeCommand("writeconfig", false);
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("When this box is checked, the plugin will deactivate during private matches.");
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		// AUTO QUEUE
		static auto queueCvar = cvarManager->getCvar(queueCvarName);
		auto queueEnabled = queueCvar.getBoolValue();

		ImGui::Text("Auto Queue");
		if (ImGui::Checkbox("", &queueEnabled)) {
			queueCvar.setValue(queueEnabled);
			cvarManager->executeCommand("writeconfig", false);
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Automatically queue when an online match has ended.");
		ImGui::SameLine();

		// DELAYED QUEUE
		static auto queueDelayCvar = cvarManager->getCvar(qDelayCvarName);
		auto queueDelayTime = queueDelayCvar.getFloatValue();

		ImGui::SameLine();

		ImGui::PushItemWidth( ImGui::GetContentRegionAvail().x * 0.5f);
		ImGui::PushID("queueDelayTime");
		if (ImGui::SliderFloat("", &queueDelayTime, 0.0f, 20.0f, "Delay: %.1f s")) {
			queueDelayCvar.setValue(queueDelayTime);
			cvarManager->executeCommand("writeconfig", false);
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Set a delay for the auto-queue.");

		ImGui::PopID();
		ImGui::PopItemWidth();
		// DISABLE Q FOR CASUAL
		static auto disableCasualQueueCvar = cvarManager->getCvar(disableCasualQCvarName);
		auto disableCasualQueue = disableCasualQueueCvar.getBoolValue();
		ImGui::PushID("disableQ");
		if (ImGui::Checkbox("Disable for Casual", &disableCasualQueue)) {
			disableCasualQueueCvar.setValue(disableCasualQueue);
			cvarManager->executeCommand("writeconfig", false);
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Disable the automatic queue for casual games.");
		ImGui::PopID();

		static auto delayCvar = cvarManager->getCvar(DelayCvarName);
		auto delayTime = delayCvar.getFloatValue();
		static auto disCasualCvar = cvarManager->getCvar(disableCasualCvarName);
		auto disableCasual = disCasualCvar.getBoolValue();

		ImGui::Text("Instant Exit");
		if (ImGui::Checkbox("Disable for Casual", &disableCasual)) {
			disCasualCvar.setValue(disableCasual);
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Disable the all automatic-exit settings for casual matches..");
		ImGui::SameLine();

		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
		if (ImGui::SliderFloat("Delay", &delayTime, 0.0f, 20.0f, "%.1f secs")) {
			delayCvar.setValue(delayTime);
			cvarManager->log("Delay Set");
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Set a delay for auto-exit options (options are below).");
		ImGui::PopItemWidth();
		ImGui::Spacing();
		ImGui::Spacing();
		//-----------------------------------------------------------------------------
		// Auto-Exit
		//-----------------------------------------------------------------------------

		ImGui::Text("Auto-Exit Options");
		ImGui::SameLine();
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Exit to desired location immediately, or with delay.\n"
							  "Current options: Main-Menu, Freeplay, Custom Training Pack.\n"
							  "Workshops, private game modes and specialty game modes are on the way.\n\n"
							  "*If multiple exit-options are enabled, the uppmost most selection will be executed.");
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();
		
		if (ImGui::TreeNode("Freeplay"))
		{
			static auto trainingCvar = cvarManager->getCvar("freeplay_enable");
			auto trainingEnabled = trainingCvar.getBoolValue();

			if (ImGui::Checkbox("", &trainingEnabled)) {
				trainingCvar.setValue(trainingEnabled);
				cvarManager->executeCommand("writeconfig", false);
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Instant-exit to Freeplay.\n");

			ImGui::TreePop();
		}
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::TreeNode("Custom Training"))
		{
			static char str0[128] = "";
			static auto ctrainingCvar = cvarManager->getCvar("custom_enable");
			auto customTrainingEnabled = ctrainingCvar.getBoolValue();
			static auto ctrainingPCvar = cvarManager->getCvar("custom_code");
			auto customtrainingPCvarName = ctrainingPCvar.getStringValue();

			static ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsNoBlank;

			if (ImGui::Checkbox("", &customTrainingEnabled)) {
				ctrainingCvar.setValue(customTrainingEnabled);
				cvarManager->executeCommand("writeconfig", false);
				cvarManager->log("Custom Training Enabled! Current training pack: " + customtrainingPCvarName);
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Instant-exit to Custom Training.\n");
			ImGui::SameLine();
			if (ImGui::InputTextWithHint("Training Pack Code", "XXXX-XXXX-XXXX-XXXX", str0, IM_ARRAYSIZE(str0), 0, 0)) {
				ctrainingPCvar.setValue(str0);
				cvarManager->executeCommand("writeconfig", false);
				cvarManager->log("New training pack code entered:");
				cvarManager->log(str0);
			}
			ImGui::SameLine();
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Enter training code pack. The default is:.\n"
						"A0FE-F860-967D-E628\n"
						"This is one I made :)"
					);
			ImGui::TreePop();
		}
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::TreeNode("Exit"))
		{
			static auto exitCvar = cvarManager->getCvar("exit_enable");
			auto exitEnabled = exitCvar.getBoolValue();
			if (ImGui::Checkbox("", &exitEnabled)) {
				exitCvar.setValue(exitEnabled);
				cvarManager->executeCommand("writeconfig", false);
				cvarManager->log("Enabled Instant Exit");
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Instant-exit to Main-Menu.\n");
			ImGui::TreePop();
		}
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::EndChild();
		ImGui::EndTabItem();
	}

}

/// <summary> Renders about window. </summary>
void PremierSuite::renderAboutWindow(bool* p_open)
{
	if (!ImGui::Begin("About PremierSuite", p_open, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}
	ImGui::Separator();
	ImGui::Text("By Flynn OConnell with help from many BakkesMod Plugin Developers and Dear ImGui documentation.");
	ImGui::Text("PremierSuite is licensed under the MIT License, see LICENSE for more information.");

	static bool show_config_info = false;
	ImGui::Checkbox("Config/Dear ImGui Information", &show_config_info);
	if (show_config_info)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();

		bool copy_to_clipboard = ImGui::Button("Copy to clipboard");
		ImGui::BeginChildFrame(ImGui::GetID("cfginfos"), ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * 18), ImGuiWindowFlags_NoMove);
		if (copy_to_clipboard)
		{
			ImGui::LogToClipboard();
			ImGui::LogText("```\n"); // Back quotes will make the text appears without formatting when pasting to GitHub
		}

		ImGui::Text("Dear ImGui %s (%d)", IMGUI_VERSION, IMGUI_VERSION_NUM);
		ImGui::Separator();
		ImGui::Text("sizeof(size_t): %d, sizeof(ImDrawIdx): %d, sizeof(ImDrawVert): %d", (int)sizeof(size_t), (int)sizeof(ImDrawIdx), (int)sizeof(ImDrawVert));
		ImGui::Text("define: __cplusplus=%d", (int)__cplusplus);
#ifdef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
		ImGui::Text("define: IMGUI_DISABLE_OBSOLETE_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS
		ImGui::Text("define: IMGUI_DISABLE_WIN32_DEFAULT_CLIPBOARD_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS
		ImGui::Text("define: IMGUI_DISABLE_WIN32_DEFAULT_IME_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_WIN32_FUNCTIONS
		ImGui::Text("define: IMGUI_DISABLE_WIN32_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS
		ImGui::Text("define: IMGUI_DISABLE_DEFAULT_FORMAT_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS
		ImGui::Text("define: IMGUI_DISABLE_DEFAULT_MATH_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS
		ImGui::Text("define: IMGUI_DISABLE_DEFAULT_FILE_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_FILE_FUNCTIONS
		ImGui::Text("define: IMGUI_DISABLE_FILE_FUNCTIONS");
#endif
#ifdef IMGUI_DISABLE_DEFAULT_ALLOCATORS
		ImGui::Text("define: IMGUI_DISABLE_DEFAULT_ALLOCATORS");
#endif
#ifdef IMGUI_USE_BGRA_PACKED_COLOR
		ImGui::Text("define: IMGUI_USE_BGRA_PACKED_COLOR");
#endif
#ifdef _WIN32
		ImGui::Text("define: _WIN32");
#endif
#ifdef _WIN64
		ImGui::Text("define: _WIN64");
#endif
#ifdef __linux__
		ImGui::Text("define: __linux__");
#endif
#ifdef __APPLE__
		ImGui::Text("define: __APPLE__");
#endif
#ifdef _MSC_VER
		ImGui::Text("define: _MSC_VER=%d", _MSC_VER);
#endif
#ifdef __MINGW32__
		ImGui::Text("define: __MINGW32__");
#endif
#ifdef __MINGW64__
		ImGui::Text("define: __MINGW64__");
#endif
#ifdef __GNUC__
		ImGui::Text("define: __GNUC__=%d", (int)__GNUC__);
#endif
#ifdef __clang_version__
		ImGui::Text("define: __clang_version__=%s", __clang_version__);
#endif
		ImGui::Separator();
		ImGui::Text("io.BackendPlatformName: %s", io.BackendPlatformName ? io.BackendPlatformName : "NULL");
		ImGui::Text("io.BackendRendererName: %s", io.BackendRendererName ? io.BackendRendererName : "NULL");
		ImGui::Text("io.ConfigFlags: 0x%08X", io.ConfigFlags);
		if (io.ConfigFlags & ImGuiConfigFlags_NavEnableKeyboard)        ImGui::Text(" NavEnableKeyboard");
		if (io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad)         ImGui::Text(" NavEnableGamepad");
		if (io.ConfigFlags & ImGuiConfigFlags_NavEnableSetMousePos)     ImGui::Text(" NavEnableSetMousePos");
		if (io.ConfigFlags & ImGuiConfigFlags_NavNoCaptureKeyboard)     ImGui::Text(" NavNoCaptureKeyboard");
		if (io.ConfigFlags & ImGuiConfigFlags_NoMouse)                  ImGui::Text(" NoMouse");
		if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)      ImGui::Text(" NoMouseCursorChange");
		if (io.MouseDrawCursor)                                         ImGui::Text("io.MouseDrawCursor");
		if (io.ConfigMacOSXBehaviors)                                   ImGui::Text("io.ConfigMacOSXBehaviors");
		if (io.ConfigInputTextCursorBlink)                              ImGui::Text("io.ConfigInputTextCursorBlink");
		if (io.ConfigWindowsResizeFromEdges)                            ImGui::Text("io.ConfigWindowsResizeFromEdges");
		if (io.ConfigWindowsMoveFromTitleBarOnly)                       ImGui::Text("io.ConfigWindowsMoveFromTitleBarOnly");
		if (io.ConfigWindowsMemoryCompactTimer >= 0.0f)                 ImGui::Text("io.ConfigWindowsMemoryCompactTimer = %.1ff", io.ConfigWindowsMemoryCompactTimer);
		ImGui::Text("io.BackendFlags: 0x%08X", io.BackendFlags);
		if (io.BackendFlags & ImGuiBackendFlags_HasGamepad)             ImGui::Text(" HasGamepad");
		if (io.BackendFlags & ImGuiBackendFlags_HasMouseCursors)        ImGui::Text(" HasMouseCursors");
		if (io.BackendFlags & ImGuiBackendFlags_HasSetMousePos)         ImGui::Text(" HasSetMousePos");
		if (io.BackendFlags & ImGuiBackendFlags_RendererHasVtxOffset)   ImGui::Text(" RendererHasVtxOffset");
		ImGui::Separator();
		ImGui::Text("io.Fonts: %d fonts, Flags: 0x%08X, TexSize: %d,%d", io.Fonts->Fonts.Size, io.Fonts->Flags, io.Fonts->TexWidth, io.Fonts->TexHeight);
		ImGui::Text("io.DisplaySize: %.2f,%.2f", io.DisplaySize.x, io.DisplaySize.y);
		ImGui::Text("io.DisplayFramebufferScale: %.2f,%.2f", io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
		ImGui::Separator();
		ImGui::Text("style.WindowPadding: %.2f,%.2f", style.WindowPadding.x, style.WindowPadding.y);
		ImGui::Text("style.WindowBorderSize: %.2f", style.WindowBorderSize);
		ImGui::Text("style.FramePadding: %.2f,%.2f", style.FramePadding.x, style.FramePadding.y);
		ImGui::Text("style.FrameRounding: %.2f", style.FrameRounding);
		ImGui::Text("style.FrameBorderSize: %.2f", style.FrameBorderSize);
		ImGui::Text("style.ItemSpacing: %.2f,%.2f", style.ItemSpacing.x, style.ItemSpacing.y);
		ImGui::Text("style.ItemInnerSpacing: %.2f,%.2f", style.ItemInnerSpacing.x, style.ItemInnerSpacing.y);

		if (copy_to_clipboard)
		{
			ImGui::LogText("\n```\n");
			ImGui::LogFinish();
		}
		ImGui::EndChildFrame();
	}
	ImGui::End();
}

/// <summary> render styles combo selector. </summary>
bool PremierSuite::renderStyleCombo(const char* label)
{
	static int style_idx = -1;
	if (ImGui::Combo(label, &style_idx, "Classic\0Dark\0Light\0DarknessEmbrace\0MinimalMatte\0DarkRed\0"))
	{
		switch (style_idx)
		{
			case 0: ImGui::StyleColorsClassic(); break;
			case 1: ImGui::StyleColorsDark(); break;
			case 2: ImGui::StyleColorsLight(); break;
			case 3: EmbraceTheDarknessTheme(); break;
			case 4: MinimalMatteTheme(); break;
			case 5: DarkRedTheme(); break;
		}
		return true;
	}
	return false;
}

//void ToggleButton(const char* str_id, bool* v)
//{
//	ImVec4* colors = ImGui::GetStyle().Colors;
//	ImVec2 p = ImGui::GetCursorScreenPos();
//	ImDrawList* draw_list = ImGui::GetWindowDrawList();
//
//	float height = ImGui::GetFrameHeight();
//	float width = height * 1.55f;
//	float radius = height * 0.50f;
//
//	ImGui::InvisibleButton(str_id, ImVec2(width, height));
//	if (ImGui::IsItemClicked()) *v = !*v;
//	ImGuiContext& gg = *GImGui;
//	float ANIM_SPEED = 0.085f;
//	if (gg.LastActiveId == gg.CurrentWindow->GetID(str_id))// && g.LastActiveIdTimer < ANIM_SPEED)
//		float t_anim = ImSaturate(gg.LastActiveIdTimer / ANIM_SPEED);
//	if (ImGui::IsItemHovered())
//		draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*v ? colors[ImGuiCol_ButtonActive] : ImVec4(0.78f, 0.78f, 0.78f, 1.0f)), height * 0.5f);
//	else
//		draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(*v ? colors[ImGuiCol_Button] : ImVec4(0.85f, 0.85f, 0.85f, 1.0f)), height * 0.50f);
//	draw_list->AddCircleFilled(ImVec2(p.x + radius + (*v ? 1 : 0) * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
//}

// Usage:
//  static ExampleAppLog my_log;
//  my_log.AddLog("Hello %d world\n", 123);
//  my_log.Draw("title");
struct ExampleAppLog
{
	ImGuiTextBuffer     Buf;
	ImGuiTextFilter     Filter;
	ImVector<int>       LineOffsets;        // Index to lines offset. We maintain this with AddLog() calls, allowing us to have a random access on lines
	bool                AutoScroll;     // Keep scrolling if already at the bottom

	ExampleAppLog()
	{
		AutoScroll = true;
		Clear();
	}

	void    Clear()
	{
		Buf.clear();
		LineOffsets.clear();
		LineOffsets.push_back(0);
	}

	void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
	{
		int old_size = Buf.size();
		va_list args;
		va_start(args, fmt);
		Buf.appendfv(fmt, args);
		va_end(args);
		for (int new_size = Buf.size(); old_size < new_size; old_size++)
			if (Buf[old_size] == '\n')
				LineOffsets.push_back(old_size + 1);
	}

	void    Draw(const char* title, bool* p_open = NULL)
	{
		if (!ImGui::Begin(title, p_open))
		{
			ImGui::End();
			return;
		}

		// Options menu
		if (ImGui::BeginPopup("Options"))
		{
			ImGui::Checkbox("Auto-scroll", &AutoScroll);
			ImGui::EndPopup();
		}

		// Main window
		if (ImGui::Button("Options"))
			ImGui::OpenPopup("Options");
		ImGui::SameLine();
		bool clear = ImGui::Button("Clear");
		ImGui::SameLine();
		bool copy = ImGui::Button("Copy");
		ImGui::SameLine();
		Filter.Draw("Filter", -100.0f);

		ImGui::Separator();
		ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

		if (clear)
			Clear();
		if (copy)
			ImGui::LogToClipboard();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		const char* buf = Buf.begin();
		const char* buf_end = Buf.end();
		if (Filter.IsActive())
		{
			// In this example we don't use the clipper when Filter is enabled.
			// This is because we don't have a random access on the result on our filter.
			// A real application processing logs with ten of thousands of entries may want to store the result of search/filter.
			// especially if the filtering function is not trivial (e.g. reg-exp).
			for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
			{
				const char* line_start = buf + LineOffsets[line_no];
				const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
				if (Filter.PassFilter(line_start, line_end))
					ImGui::TextUnformatted(line_start, line_end);
			}
		}
		else
		{
			// The simplest and easy way to display the entire buffer:
			//   ImGui::TextUnformatted(buf_begin, buf_end);
			// And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward to skip non-visible lines.
			// Here we instead demonstrate using the clipper to only process lines that are within the visible area.
			// If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them on your side is recommended.
			// Using ImGuiListClipper requires A) random access into your data, and B) items all being the  same height,
			// both of which we can handle since we an array pointing to the beginning of each line of text.
			// When using the filter (in the block of code above) we don't have random access into the data to display anymore, which is why we don't use the clipper.
			// Storing or skimming through the search result would make it possible (and would be recommended if you want to search through tens of thousands of entries)
			ImGuiListClipper clipper;
			clipper.Begin(LineOffsets.Size);
			while (clipper.Step())
			{
				for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
				{
					const char* line_start = buf + LineOffsets[line_no];
					const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
					ImGui::TextUnformatted(line_start, line_end);
				}
			}
			clipper.End();
		}
		ImGui::PopStyleVar();

		if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(1.0f);

		ImGui::EndChild();
		ImGui::End();
	}
};

struct ExampleAppConsole
{
	char                  InputBuf[256];
	ImVector<char*>       Items;
	ImVector<const char*> Commands;
	ImVector<char*>       History;
	int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
	ImGuiTextFilter       Filter;
	bool                  AutoScroll;
	bool                  ScrollToBottom;

	ExampleAppConsole()
	{
		ClearLog();
		memset(InputBuf, 0, sizeof(InputBuf));
		HistoryPos = -1;
		Commands.push_back("HELP");
		Commands.push_back("HISTORY");
		Commands.push_back("CLEAR");
		Commands.push_back("CLASSIFY");  // "classify" is only here to provide an example of "C"+[tab] completing to "CL" and displaying matches.
		AutoScroll = true;
		ScrollToBottom = false;
		AddLog("Welcome to Dear ImGui!");
	}
	~ExampleAppConsole()
	{
		ClearLog();
		for (int i = 0; i < History.Size; i++)
			free(History[i]);
	}

	// Portable helpers
	static int   Stricmp(const char* str1, const char* str2) { int d; while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; } return d; }
	static int   Strnicmp(const char* str1, const char* str2, int n) { int d = 0; while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; n--; } return d; }
	static char* Strdup(const char* str) { size_t len = strlen(str) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)str, len); }
	static void  Strtrim(char* str) { char* str_end = str + strlen(str); while (str_end > str && str_end[-1] == ' ') str_end--; *str_end = 0; }

	void    ClearLog()
	{
		for (int i = 0; i < Items.Size; i++)
			free(Items[i]);
		Items.clear();
	}

	void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
	{
		// FIXME-OPT
		char buf[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
		buf[IM_ARRAYSIZE(buf) - 1] = 0;
		va_end(args);
		Items.push_back(Strdup(buf));
	}

	void    Draw(const char* title, bool* p_open)
	{
		ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin(title, p_open))
		{
			ImGui::End();
			return;
		}

		// As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar. So e.g. IsItemHovered() will return true when hovering the title bar.
		// Here we create a context menu only available from the title bar.
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Close Console"))
				*p_open = false;
			ImGui::EndPopup();
		}

		ImGui::TextWrapped("This example implements a console with basic coloring, completion and history. A more elaborate implementation may want to store entries along with extra data such as timestamp, emitter, etc.");
		ImGui::TextWrapped("Enter 'HELP' for help, press TAB to use text completion.");

		// TODO: display items starting from the bottom

		if (ImGui::SmallButton("Add Dummy Text")) { AddLog("%d some text", Items.Size); AddLog("some more text"); AddLog("display very important message here!"); } ImGui::SameLine();
		if (ImGui::SmallButton("Add Dummy Error")) { AddLog("[error] something went wrong"); } ImGui::SameLine();
		if (ImGui::SmallButton("Clear")) { ClearLog(); } ImGui::SameLine();
		bool copy_to_clipboard = ImGui::SmallButton("Copy");
		//static float t = 0.0f; if (ImGui::GetTime() - t > 0.02f) { t = ImGui::GetTime(); AddLog("Spam %f", t); }

		ImGui::Separator();

		// Options menu
		if (ImGui::BeginPopup("Options"))
		{
			ImGui::Checkbox("Auto-scroll", &AutoScroll);
			ImGui::EndPopup();
		}

		// Options, Filter
		if (ImGui::Button("Options"))
			ImGui::OpenPopup("Options");
		ImGui::SameLine();
		Filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
		ImGui::Separator();

		const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); // 1 separator, 1 input text
		ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar); // Leave room for 1 separator + 1 InputText
		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::Selectable("Clear")) ClearLog();
			ImGui::EndPopup();
		}

		// Display every line as a separate entry so we can change their color or add custom widgets. If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
		// NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping to only process visible items.
		// You can seek and display only the lines that are visible using the ImGuiListClipper helper, if your elements are evenly spaced and you have cheap random access to the elements.
		// To use the clipper we could replace the 'for (int i = 0; i < Items.Size; i++)' loop with:
		//     ImGuiListClipper clipper(Items.Size);
		//     while (clipper.Step())
		//         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		// However, note that you can not use this code as is if a filter is active because it breaks the 'cheap random-access' property. We would need random-access on the post-filtered list.
		// A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices that passed the filtering test, recomputing this array when user changes the filter,
		// and appending newly elements as they are inserted. This is left as a task to the user until we can manage to improve this example code!
		// If your items are of variable size you may want to implement code similar to what ImGuiListClipper does. Or split your data into fixed height items to allow random-seeking into your list.
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
		if (copy_to_clipboard)
			ImGui::LogToClipboard();
		for (int i = 0; i < Items.Size; i++)
		{
			const char* item = Items[i];
			if (!Filter.PassFilter(item))
				continue;

			// Normally you would store more information in your item (e.g. make Items[] an array of structure, store color/type etc.)
			bool pop_color = false;
			if (strstr(item, "[error]")) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f)); pop_color = true; }
			else if (strncmp(item, "# ", 2) == 0) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.6f, 1.0f)); pop_color = true; }
			ImGui::TextUnformatted(item);
			if (pop_color)
				ImGui::PopStyleColor();
		}
		if (copy_to_clipboard)
			ImGui::LogFinish();

		if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
			ImGui::SetScrollHereY(1.0f);
		ScrollToBottom = false;

		ImGui::PopStyleVar();
		ImGui::EndChild();
		ImGui::Separator();

		// Command-line
		bool reclaim_focus = false;
		if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory, &TextEditCallbackStub, (void*)this))
		{
			char* s = InputBuf;
			Strtrim(s);
			if (s[0])
				ExecCommand(s);
			strcpy(s, "");
			reclaim_focus = true;
		}

		// Auto-focus on window apparition
		ImGui::SetItemDefaultFocus();
		if (reclaim_focus)
			ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

		ImGui::End();
	}

	void    ExecCommand(const char* command_line)
	{
		AddLog("# %s\n", command_line);

		// Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
		HistoryPos = -1;
		for (int i = History.Size - 1; i >= 0; i--)
			if (Stricmp(History[i], command_line) == 0)
			{
				free(History[i]);
				History.erase(History.begin() + i);
				break;
			}
		History.push_back(Strdup(command_line));

		// Process command
		if (Stricmp(command_line, "CLEAR") == 0)
		{
			ClearLog();
		}
		else if (Stricmp(command_line, "HELP") == 0)
		{
			AddLog("Commands:");
			for (int i = 0; i < Commands.Size; i++)
				AddLog("- %s", Commands[i]);
		}
		else if (Stricmp(command_line, "HISTORY") == 0)
		{
			int first = History.Size - 10;
			for (int i = first > 0 ? first : 0; i < History.Size; i++)
				AddLog("%3d: %s\n", i, History[i]);
		}
		else
		{
			AddLog("Unknown command: '%s'\n", command_line);
		}

		// On commad input, we scroll to bottom even if AutoScroll==false
		ScrollToBottom = true;
	}

	static int TextEditCallbackStub(ImGuiInputTextCallbackData* data) // In C++11 you are better off using lambdas for this sort of forwarding callbacks
	{
		ExampleAppConsole* console = (ExampleAppConsole*)data->UserData;
		return console->TextEditCallback(data);
	}

	int     TextEditCallback(ImGuiInputTextCallbackData* data)
	{
		//AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
		switch (data->EventFlag)
		{
		case ImGuiInputTextFlags_CallbackCompletion:
		{
			// Example of TEXT COMPLETION

			// Locate beginning of current word
			const char* word_end = data->Buf + data->CursorPos;
			const char* word_start = word_end;
			while (word_start > data->Buf)
			{
				const char c = word_start[-1];
				if (c == ' ' || c == '\t' || c == ',' || c == ';')
					break;
				word_start--;
			}

			// Build a list of candidates
			ImVector<const char*> candidates;
			for (int i = 0; i < Commands.Size; i++)
				if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
					candidates.push_back(Commands[i]);

			if (candidates.Size == 0)
			{
				// No match
				AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
			}
			else if (candidates.Size == 1)
			{
				// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
				data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
				data->InsertChars(data->CursorPos, candidates[0]);
				data->InsertChars(data->CursorPos, " ");
			}
			else
			{
				// Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
				int match_len = (int)(word_end - word_start);
				for (;;)
				{
					int c = 0;
					bool all_candidates_matches = true;
					for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
						if (i == 0)
							c = toupper(candidates[i][match_len]);
						else if (c == 0 || c != toupper(candidates[i][match_len]))
							all_candidates_matches = false;
					if (!all_candidates_matches)
						break;
					match_len++;
				}

				if (match_len > 0)
				{
					data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
					data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
				}

				// List matches
				AddLog("Possible matches:\n");
				for (int i = 0; i < candidates.Size; i++)
					AddLog("- %s\n", candidates[i]);
			}

			break;
		}
		case ImGuiInputTextFlags_CallbackHistory:
		{
			// Example of HISTORY
			const int prev_history_pos = HistoryPos;
			if (data->EventKey == ImGuiKey_UpArrow)
			{
				if (HistoryPos == -1)
					HistoryPos = History.Size - 1;
				else if (HistoryPos > 0)
					HistoryPos--;
			}
			else if (data->EventKey == ImGuiKey_DownArrow)
			{
				if (HistoryPos != -1)
					if (++HistoryPos >= History.Size)
						HistoryPos = -1;
			}

			// A better implementation would preserve the data on the current input line along with cursor position.
			if (prev_history_pos != HistoryPos)
			{
				const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
				data->DeleteChars(0, data->BufTextLen);
				data->InsertChars(0, history_str);
			}
		}
		}
		return 0;
	}
};

// Demonstrate creating a simple log window with basic filtering.
static void ShowExampleAppLog(bool* p_open)
{
	static ExampleAppLog log;

	// For the demo: add a debug button _BEFORE_ the normal log window contents
	// We take advantage of a rarely used feature: multiple calls to Begin()/End() are appending to the _same_ window.
	// Most of the contents of the window will be added by the log.Draw() call.
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
	ImGui::Begin("Example: Log", p_open);
	if (ImGui::SmallButton("[Debug] Add 5 entries"))
	{
		static int counter = 0;
		for (int n = 0; n < 5; n++)
		{
			const char* categories[3] = { "info", "warn", "error" };
			const char* words[] = { "Bumfuzzled", "Cattywampus", "Snickersnee", "Abibliophobia", "Absquatulate", "Nincompoop", "Pauciloquent" };
			log.AddLog("[%05d] [%s] Hello, current time is %.1f, here's a word: '%s'\n",
				ImGui::GetFrameCount(), categories[counter % IM_ARRAYSIZE(categories)], ImGui::GetTime(), words[counter % IM_ARRAYSIZE(words)]);
			counter++;
		}
	}
	ImGui::End();

	// Actually call in the regular Log helper (which will Begin() into the same window as we just did)
	log.Draw("Example: Log", p_open);
}

static void ShowExampleAppConsole(bool* p_open)
{
	static ExampleAppConsole console;
	console.Draw("Example: Console", p_open);
}

//-----------------------------------------------------------------------------
// GUI Menu / Overlay Helpers
//-----------------------------------------------------------------------------

/// <summary>Gets the menu name.</summary>
/// <returns>The menu name</returns>
std::string PremierSuite::GetMenuName()
{
	return "premiersuite";
}

/// <summary>Gets the menu title.</summary>
/// <returns>The menu title</returns>
std::string PremierSuite::GetMenuTitle()
{
	return menuTitle;
}

/// <summary>Sets the ImGui context.</summary>
void PremierSuite::SetImGuiContext(const uintptr_t ctx)
{
	ImGuiContext* context = reinterpret_cast<ImGuiContext*>(ctx);
	ImGui::SetCurrentContext(context);
}

/// <summary>Gets if the user input should be blocked.</summary>
/// <remarks>This returns true when the user is using the widgets.</remarks>
/// <returns>Bool with if the user input should be blocked</returns>
bool PremierSuite::ShouldBlockInput()
{
	return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
}

/// <summary>Gets if it is an active overlay.</summary>
/// <remarks>This always returns true.</remarks>
/// <returns>Bool with if it is an active overlay</returns>
bool PremierSuite::IsActiveOverlay()
{
	return true;
}

/// <summary>Sets the window to open.</summary>
void PremierSuite::OnOpen()
{
	isWindowOpen = true;
}

/// <summary>Sets the window to close.</summary>
void PremierSuite::OnClose()
{
	isWindowOpen = false;
}


///
/// WORKSHOP: BETA
///
//
//ImGui::TreeNode("#Workshop"); {
//
//	static auto workshopCvar = cvarManager->getCvar(wtrainingCvarName);
//	auto enableWorkshopMaps = workshopCvar.getBoolValue();
//	static auto workshopMapCvar = cvarManager->getCvar(workshopCvarName);
//	std::string workshopMapCvarString = workshopCvar.getStringValue();
//
//	if (gameWrapper->IsUsingSteamVersion()) {
//		ImGui::SameLine();
//		if (ImGui::Checkbox("Enable workshop maps", &enableWorkshopMaps)) {
//			currentMap.clear();
//			refreshCustomMapPaths = true;
//		}
//	}
//	ImGui::SameLine();
//	if (ImGui::Checkbox("Enable custom maps", &enableCustomMaps)) {
//		currentMap.clear();
//		refreshCustomMapPaths = true;
//	}
//	if (!enableWorkshopMaps && !enableCustomMaps) {
//		if (ImGui::SearchableCombo("##Maps", currentMap, maps, "No maps found", "type to search")) {
//			workshopMapCvar.setValue(workshopMapCvarString);
//			cvarManager->executeCommand("writeconfig", false);
//			cvarManager->log("no map selected");
//		}
//	}
//	else {
//		std::filesystem::path currentMapPath = currentMap;
//		if (renderCustomMapsSelection(customMapPaths, currentMapPath, refreshCustomMapPaths, enableWorkshopMaps, enableCustomMaps))
//		{
//			currentMap = currentMapPath.string();
//			const std::filesystem::path config = currentMapPath.replace_extension(L".cfg");
//
//		}
//	}
//
//	ImGui::TreePop();
//}
//
///<summary>Renders Workshop and Custom Maps.</summary>
//bool PremierSuite::renderCustomMapsSelection(std::map<std::filesystem::path, std::string>& customMaps, std::filesystem::path& currentCustomMap,
//					     bool& refreshCustomMaps, const bool includeWorkshopMaps, const bool includeCustomMaps) {
//	if (refreshCustomMaps) {
//		customMaps.clear();
//		if (includeWorkshopMaps && gameWrapper->IsUsingSteamVersion()) {
//			bool addedWorkshopMaps = false;
//			customMaps.insert({ *workshopMapDirPath, "#Workshop Maps:" });
//			for (const std::filesystem::path& workshopMap : getWorkshopMaps(*workshopMapDirPath)) {
//				addedWorkshopMaps = true;
//				if (auto it = subscribedWorkshopMaps.find(
//					std::strtoull(workshopMap.parent_path().stem().string().c_str(), nullptr, 10));
//					it != subscribedWorkshopMaps.end() && !it->second.Title.empty()) {
//					customMaps.insert({ workshopMap, it->second.Title });
//				}
//				else {
//					customMaps.insert({ workshopMap, workshopMap.stem().u8string() });
//				}
//			}
//			if (!addedWorkshopMaps) {
//				customMaps.erase(*workshopMapDirPath);
//			}
//		}
//		if (includeCustomMaps) {
//			bool addedCustomMaps = false;
//			customMaps.insert({ *customMapDirPath, "#Custom Maps:" });
//			for (const std::filesystem::path& customMap : GetFilesFromDir(*customMapDirPath, 2, ".upk", ".udk")) {
//				addedCustomMaps = true;
//				if (auto it = subscribedWorkshopMaps.find(
//					std::strtoull(customMap.parent_path().stem().string().c_str(), nullptr, 10));
//					it != subscribedWorkshopMaps.end() && !it->second.Title.empty()) {
//					customMaps.insert({ customMap, it->second.Title });
//				}
//				else {
//					customMaps.insert({ customMap, customMap.stem().u8string() });
//				}
//			}
//			if (!addedCustomMaps) {
//				customMaps.erase(*customMapDirPath);
//			}
//			bool addedCopiedMaps = false;
//			customMaps.insert({ COPIED_MAPS_PATH, "#Copied Maps:" });
//			for (const std::filesystem::path& customMap : GetFilesFromDir(COPIED_MAPS_PATH, 1, ".upk")) {
//				addedCopiedMaps = true;
//				customMaps.insert({ customMap, customMap.stem().u8string() });
//			}
//			if (!addedCopiedMaps) {
//				customMaps.erase(COPIED_MAPS_PATH);
//			}
//		}
//		refreshCustomMaps = false;
//	}
//
//	bool valueChanged = false;
//	if (ImGui::SearchableCombo("##Maps", currentCustomMap, customMaps, "No maps found", "type to search")) {
//		valueChanged = true;
//
//	}
//
//	return valueChanged;
//}
//
