#include "pch.h"
#include "{{cookiecutter.plugin_name}}.h"

/* Plugin Settings Window code here
std::string {{cookiecutter.plugin_name}}::GetPluginName() {
	return "{{cookiecutter.plugin_name}}";
}

void {{cookiecutter.plugin_name}}::SetImGuiContext(uintptr_t ctx) {
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

// Render the plugin settings here
// This will show up in bakkesmod when the plugin is loaded at
//  f2 -> plugins -> {{cookiecutter.plugin_name}}
void {{cookiecutter.plugin_name}}::RenderSettings() {
	ImGui::TextUnformatted("{{cookiecutter.plugin_name}} plugin settings");
}
*/

/*
// Do ImGui rendering here
void {{cookiecutter.plugin_name}}::Render()
{
	if (!ImGui::Begin(menuTitle_.c_str(), &isWindowOpen_, ImGuiWindowFlags_None))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}

	ImGui::End();

	if (!isWindowOpen_)
	{
		cvarManager->executeCommand("togglemenu " + GetMenuName());
	}
}

// Name of the menu that is used to toggle the window.
std::string {{cookiecutter.plugin_name}}::GetMenuName()
{
	return "{{cookiecutter.plugin_name|lower}}";
}

// Title to give the menu
std::string {{cookiecutter.plugin_name}}::GetMenuTitle()
{
	return menuTitle_;
}

// Don't call this yourself, BM will call this function with a pointer to the current ImGui context
void {{cookiecutter.plugin_name}}::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

// Should events such as mouse clicks/key inputs be blocked so they won't reach the game
bool {{cookiecutter.plugin_name}}::ShouldBlockInput()
{
	return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
}

// Return true if window should be interactive
bool {{cookiecutter.plugin_name}}::IsActiveOverlay()
{
	return true;
}

// Called when window is opened
void {{cookiecutter.plugin_name}}::OnOpen()
{
	isWindowOpen_ = true;
}

// Called when window is closed
void {{cookiecutter.plugin_name}}::OnClose()
{
	isWindowOpen_ = false;
}
*/
