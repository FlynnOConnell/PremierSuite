#include "pch.h"
#include "PremierSuite.h"
#include "IMGUI/imgui.h"
#include "IMGUI/imgui_internal.h"

#define IM_COL32_ERROR_RED (ImColor)IM_COL32(204,0,0,255)

/*
 *  Defining Functions
 */

/// <summary>ImGui widgets to render.</summary>
void PremierSuite::Render()
{
	static bool show_app_main_menu_bar = true;
	if (ImGui::Begin(menuTitle.c_str(), &isWindowOpen, ImGuiWindowFlags_AlwaysAutoResize))
	{
		if (ImGui::BeginTabBar("#TabBar", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_NoTooltip)) {
			renderInstantSettingsTab();
			renderKeybindsTab();
			ImGui::EndTabBar();
		}
	}
	ImGui::End();

	if (!isWindowOpen) {
		cvarManager->executeCommand("togglemenu " + GetMenuName());
	}
}

/// <summary>Gets the menu name.</summary>
/// <returns>The menu name</returns>
std::string PremierSuite::GetMenuName()
{
	return "PremierSuite";
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


/*
 *	ImGui Widgets
*/


/// <summary>Main Settings Tab.</summary>
void PremierSuite::renderInstantSettingsTab()
{
	ImGui::StyleColorsDark();
	if (ImGui::BeginTabItem("Settings"), ImGuiWindowFlags_NoNavInputs)
	{ 
		if (ImGui::BeginChild("Main Settings"))
		{
			ImGui::Text(
				"Instant Suite: Instant Access to Queue, Freeplay, and Custom Training.\n"
				"Join discord for questions and feature requests.\n"
			);
			ImGui::Separator();
			ImGui::Indent(5);
			ImGui::Spacing();

			// ENABLE PLUGIN
			static auto pluginCvar = cvarManager->getCvar(enabledCvarName);
			auto pluginEnabled = pluginCvar.getBoolValue();

			if (ImGui::Checkbox("Enable Plugin", &pluginEnabled)) {
				pluginCvar.setValue(pluginEnabled);
				cvarManager->log("Plugin Enabled!");
			}


			// DISABLE PRIVATE
			static auto disablePrivateCvar = cvarManager->getCvar(disablePrivateCvarName);
			auto disablePrivate = disablePrivateCvar.getBoolValue();
			if (ImGui::Checkbox("Disable Plugin for Private", &disablePrivate)) {
				disablePrivateCvar.setValue(disablePrivate);
				cvarManager->executeCommand("writeconfig", false);
				cvarManager->log("Disabled Instant Functions for Private Matches");
			}
			ImGui::Separator();

			// AUTO QUEUE
			static auto queueCvar = cvarManager->getCvar(queueCvarName);
			auto queueEnabled = queueCvar.getBoolValue();

			ImGui::Text("Instant Queue Settings");
			if (ImGui::Checkbox("Enable", &queueEnabled)) {
				queueCvar.setValue(queueEnabled);
				cvarManager->executeCommand("writeconfig", false);
				cvarManager->log("Instant Queue Enabled!");
			}

			// DELAYED QUEUE
			static auto queueDelayCvar = cvarManager->getCvar(qDelayCvarName);
			auto queueDelayTime = queueDelayCvar.getFloatValue();

			ImGui::SameLine();	
			ImGui::PushItemWidth(200);
			ImGui::PushID("queueDelayTime");
			if (ImGui::SliderFloat("Delay", &queueDelayTime, 0.0f, 20.0f, "%.1f secs")) {
				queueDelayCvar.setValue(queueDelayTime);
				cvarManager->executeCommand("writeconfig", false);
				cvarManager->log("Change Queue Delay Timer");
			}
			ImGui::PopID();
			ImGui::PopItemWidth();

			// DISABLE CASUAL
			static auto disableCasualQueueCvar = cvarManager->getCvar(disableCasualQCvarName);
			auto disableCasualQueue = disableCasualQueueCvar.getBoolValue();
			ImGui::PushID("disableQ");
			if (ImGui::Checkbox("Disable Queue for Casual", &disableCasualQueue)) {
				disableCasualQueueCvar.setValue(disableCasualQueue);
				cvarManager->executeCommand("writeconfig", false);
				cvarManager->log("Disabled Instant Functions for Private Matches");
			}
			ImGui::PopID();

			static auto delayCvar = cvarManager->getCvar(DelayCvarName);
			auto delayTime = delayCvar.getFloatValue();
			static auto disCasualCvar = cvarManager->getCvar(disableCasualCvarName);
			auto disableCasual = disCasualCvar.getBoolValue();

			ImGui::Text("Delay and Disable Casual for Exit Settings");
			if (ImGui::Checkbox("Disable for Casual", &disableCasual)) {
				disCasualCvar.setValue(disableCasual);
				cvarManager->log("Disabled for Casual");
			}
			ImGui::SameLine();
			ImGui::PushItemWidth(200);

			if (ImGui::SliderFloat("Delay", &delayTime, 0.0f, 20.0f, "%.1f secs")) {
				delayCvar.setValue(delayTime);
				cvarManager->log("Delay Set");
			}
			ImGui::PopItemWidth();
		

		ImGui::Text("Instant Exit:");

		ImGui::SameLine();
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Selection works in order, top to bottom, only if plugin is enabled.");
		ImGui::Separator();
		ImGui::Spacing();

		if (ImGui::TreeNode("Freeplay"))
		{
			static auto trainingCvar = cvarManager->getCvar(trainingCvarName);
			auto trainingEnabled = trainingCvar.getBoolValue();
			ImGui::Text("Instant Training Settings");
			if (ImGui::Checkbox("##Enable", &trainingEnabled)) {
				trainingCvar.setValue(trainingEnabled);
				cvarManager->executeCommand("writeconfig", false);
				cvarManager->log("Instant Training Enabled!");
			}
			ImGui::TreePop();
		}

		ImGui::Separator();
		ImGui::Spacing();

		if (ImGui::TreeNode("Custom Training"))
		{
			static char str0[128] = "";
			static auto ctrainingCvar = cvarManager->getCvar(ctrainingCvarName);
			auto customTrainingEnabled = ctrainingCvar.getBoolValue();
			static auto ctrainingPCvar = cvarManager->getCvar(customtrainingCvarName);
			auto customtrainingPCvarName = ctrainingPCvar.getStringValue();

			static ImGuiInputTextFlags flags = ImGuiInputTextFlags_CharsNoBlank;

			ImGui::Text("Instant Custom Training Settings");
			if (ImGui::Checkbox("##Enable", &customTrainingEnabled)) {
				ctrainingCvar.setValue(customTrainingEnabled);
				cvarManager->executeCommand("writeconfig", false);
				cvarManager->log("Custom Training Enabled! Current training pack: " + customtrainingPCvarName);
			}

			ImGui::SameLine();
			ImGui::PushItemWidth(200);
			if (ImGui::InputTextWithHint("Training Pack Code", "XXXX-XXXX-XXXX-XXXX", str0, IM_ARRAYSIZE(str0), 0, 0)) {
				ctrainingPCvar.setValue(str0);
				cvarManager->executeCommand("writeconfig", false);
				cvarManager->log("New training pack code entered:");
				cvarManager->log(str0);

			}
			ImGui::PopItemWidth();
			ImGui::TreePop();
		}
		ImGui::Separator();
		ImGui::Spacing();

		if (ImGui::TreeNode("Exit"))
		{
			static auto exitCvar = cvarManager->getCvar(exitCvarName);
			auto exitEnabled = exitCvar.getBoolValue();

			ImGui::Text("Instant Exit Settings");
			if (ImGui::Checkbox("##Enable", &exitEnabled)) {
				exitCvar.setValue(exitEnabled);
				cvarManager->executeCommand("writeconfig", false);
				cvarManager->log("Enabled Instant Exit");
			}
			ImGui::TreePop();
		}
		}
		ImGui::EndChild();
		ImGui::EndTabItem();
	}
}
	
	
/// <summary>
/// Renders training packs tab. User input's code to be saved to favorite training packs. 
/// </summary>
void PremierSuite::renderKeybindsTab()
{
	if (ImGui::BeginTabItem("Keybinds"))
	{
		if (ImGui::BeginChild("##Keybinds", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 260), false, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text(
				"Easily Change keybinds to whatever button you'd like.\n"
				"Join discord for questions and feature requests.\n"
			);
			ImGui::Separator();
			ImGui::Indent(5);
			ImGui::Spacing();

			ImGui::PushItemWidth(200);
			static char keybind[128] = "";
			static auto keybindCvar = cvarManager->getCvar(keybindCvarName);
			static char keyholder[128] = "";
			static auto keyholderCvar = cvarManager->getCvar(keyholderCvarName);

			if (ImGui::InputTextWithHint("Keybind to unbind", "Removes this key from any keybind", keyholder, IM_ARRAYSIZE(keyholder))){
				keyholderCvar.setValue(keyholder);
			}
			if (ImGui::Button("Unbind Key")){
				unbindKeybind();
				cvarManager->executeCommand("writeconfig", false);
			}
			ImGui::Separator();
			if (ImGui::InputTextWithHint("Desired keybind", "Type desired keybind here", keybind, IM_ARRAYSIZE(keybind))) {
				keybindCvar.setValue(keybind);
			}
			if (ImGui::Button("Change GUI Keybind")){
				changeGuiKeybind();
				cvarManager->executeCommand("writeconfig", false);
			}
			ImGui::PopItemWidth();
		}
		ImGui::EndChild();
		ImGui::EndTabItem();
	}
}




///
/// WORKSHOP: BETA
///

ImGui::TreeNode("#Workshop"); {

	static auto workshopCvar = cvarManager->getCvar(wtrainingCvarName);
	auto enableWorkshopMaps = workshopCvar.getBoolValue();
	static auto workshopMapCvar = cvarManager->getCvar(workshopCvarName);
	std::string workshopMapCvarString = workshopCvar.getStringValue();

	if (gameWrapper->IsUsingSteamVersion()) {
		ImGui::SameLine();
		if (ImGui::Checkbox("Enable workshop maps", &enableWorkshopMaps)) {
			currentMap.clear();
			refreshCustomMapPaths = true;
		}
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Enable custom maps", &enableCustomMaps)) {
		currentMap.clear();
		refreshCustomMapPaths = true;
	}
	if (!enableWorkshopMaps && !enableCustomMaps) {
		if (ImGui::SearchableCombo("##Maps", currentMap, maps, "No maps found", "type to search")) {
			workshopMapCvar.setValue(workshopMapCvarString);
			cvarManager->executeCommand("writeconfig", false);
			cvarManager->log("no map selected");
		}
	}
	else {
		std::filesystem::path currentMapPath = currentMap;
		if (renderCustomMapsSelection(customMapPaths, currentMapPath, refreshCustomMapPaths, enableWorkshopMaps, enableCustomMaps))
		{
			currentMap = currentMapPath.string();
			const std::filesystem::path config = currentMapPath.replace_extension(L".cfg");

		}
	}

	ImGui::TreePop();
}

/<summary>Renders Workshop and Custom Maps.</summary>
bool PremierSuite::renderCustomMapsSelection(std::map<std::filesystem::path, std::string>& customMaps, std::filesystem::path& currentCustomMap,
					     bool& refreshCustomMaps, const bool includeWorkshopMaps, const bool includeCustomMaps) {
	if (refreshCustomMaps) {
		customMaps.clear();
		if (includeWorkshopMaps && gameWrapper->IsUsingSteamVersion()) {
			bool addedWorkshopMaps = false;
			customMaps.insert({ *workshopMapDirPath, "#Workshop Maps:" });
			for (const std::filesystem::path& workshopMap : getWorkshopMaps(*workshopMapDirPath)) {
				addedWorkshopMaps = true;
				if (auto it = subscribedWorkshopMaps.find(
					std::strtoull(workshopMap.parent_path().stem().string().c_str(), nullptr, 10));
					it != subscribedWorkshopMaps.end() && !it->second.Title.empty()) {
					customMaps.insert({ workshopMap, it->second.Title });
				}
				else {
					customMaps.insert({ workshopMap, workshopMap.stem().u8string() });
				}
			}
			if (!addedWorkshopMaps) {
				customMaps.erase(*workshopMapDirPath);
			}
		}
		if (includeCustomMaps) {
			bool addedCustomMaps = false;
			customMaps.insert({ *customMapDirPath, "#Custom Maps:" });
			for (const std::filesystem::path& customMap : GetFilesFromDir(*customMapDirPath, 2, ".upk", ".udk")) {
				addedCustomMaps = true;
				if (auto it = subscribedWorkshopMaps.find(
					std::strtoull(customMap.parent_path().stem().string().c_str(), nullptr, 10));
					it != subscribedWorkshopMaps.end() && !it->second.Title.empty()) {
					customMaps.insert({ customMap, it->second.Title });
				}
				else {
					customMaps.insert({ customMap, customMap.stem().u8string() });
				}
			}
			if (!addedCustomMaps) {
				customMaps.erase(*customMapDirPath);
			}
			bool addedCopiedMaps = false;
			customMaps.insert({ COPIED_MAPS_PATH, "#Copied Maps:" });
			for (const std::filesystem::path& customMap : GetFilesFromDir(COPIED_MAPS_PATH, 1, ".upk")) {
				addedCopiedMaps = true;
				customMaps.insert({ customMap, customMap.stem().u8string() });
			}
			if (!addedCopiedMaps) {
				customMaps.erase(COPIED_MAPS_PATH);
			}
		}
		refreshCustomMaps = false;
	}

	bool valueChanged = false;
	if (ImGui::SearchableCombo("##Maps", currentCustomMap, customMaps, "No maps found", "type to search")) {
		valueChanged = true;

	}

	return valueChanged;
}




