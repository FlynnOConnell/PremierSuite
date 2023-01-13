#include "pch.h"
#include "PremierSuite.h"

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

std::string PremierSuite::GetPluginName()
{
	return "PremierSuite";
}

std::string PremierSuite::GetMenuName()
{
	return "PremierSuite";
}

std::string PremierSuite::GetMenuTitle()
{
	return menuTitle_;
}

void PremierSuite::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

bool PremierSuite::ShouldBlockInput()
{
	return ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
}

bool PremierSuite::IsActiveOverlay()
{
	return true;
}

void PremierSuite::OnOpen()
{
	isWindowOpen_ = true;
}

void PremierSuite::OnClose()
{
	isWindowOpen_ = false;
}

void PremierSuite::RenderSettings()
{
	//isSettings = true;
	//isGui = false;
	//SetWidgetSizes();
	//if (myRoboFont) ImGui::PushFont(myRoboFont);
	ImGui::BeginTabBar("#TabBar", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_NoTooltip);
	ImGui::Indent(5);
	ImGui::Spacing();
	renderSettingsTab();
	renderKeybindsTab();
	ImGui::EndTabBar();

	//if (myRoboFont) ImGui::PopFont();
}

void PremierSuite::Render()
{
	//ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver, ImVec2(0.5, 0.5));
	//ImGui::SetNextWindowSize(ImVec2(580, 555), ImGuiCond_FirstUseEver);

	if (!ImGui::Begin(menuTitle_.c_str(), &isWindowOpen_, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::End();
		return;
	}
	//isSettings = false;
	//isGui = true;
	//SetWidgetSizes();

	//if (myRoboFont) ImGui::PushFont(myRoboFont);
	if (ImGui::Begin(menuTitle_.c_str(), &isWindowOpen_, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_AlwaysAutoResize)) {

		if (ImGui::BeginMenuBar()) {
			renderMenu();
			ImGui::EndMenuBar();
		}

		ImGui::BeginTabBar("#TabBar", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_NoTooltip);
		ImGui::Indent(5);
		ImGui::Spacing();
		renderSettingsTab();
		renderKeybindsTab();
		ImGui::EndTabBar();
		ImGui::End(); // make sure this is within Begin() block, or alt-tabbing will crash due to EndChild() mismatch!
	}
	//if (myRoboFont) ImGui::PopFont();
	ImGui::End();

	if (!isWindowOpen_) {
		cvarManager->executeCommand("togglemenu PremierSuite");
		return;
	}
}

/// <summary> Renders main menu. </summary>
void PremierSuite::renderMenu()
{
	static bool show_app_console = false;
	static bool show_app_log = false;
	static bool show_about_window = false;
	static bool show_demo_window = false;
	if (show_demo_window)             ImGui::ShowDemoWindow(&show_demo_window);

	// Dear ImGui Apps (accessible from the "Tools" menu)
	static bool show_app_metrics = false;
	static bool show_app_style_editor = false;
	static bool show_app_about = false;

	ImGuiStyle& style = ImGui::GetStyle();

	if (show_app_about) { renderAboutWindow(&show_app_about); }
	if (show_app_metrics) { ImGui::ShowMetricsWindow(&show_app_metrics); }
	const bool has_debug_tools = true;
	if (ImGui::BeginMenu("Tools")) {
		HelpMarker("Tools for developers.");
		ImGui::MenuItem("Metrics/Debugger", NULL, &show_app_metrics, has_debug_tools);
		ImGui::MenuItem("Console", NULL, &show_app_console);
		ImGui::MenuItem("Log", NULL, &show_app_log);
		ImGui::MenuItem("ImGui Demo", NULL, &show_demo_window);
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("About"))
	{
		ImGui::MenuItem("About PremierSuite", NULL, &show_app_about);
		ImGui::EndMenu();
	}
	ImGui::Indent(5);
	ImGui::Spacing();
}

/// <summary> Renders keybinds tab for changing GUI keybinds. </summary>
void PremierSuite::renderKeybindsTab()
{
	//ImGuiIO& io = ImGui::GetIO();
	if (ImGui::BeginTabItem("Keybinds")) {
		if (ImGui::BeginChild("##Keybinds")) {

			ImGui::Dummy(ImVec2(0.0f, 5.0f));

			ImGui::Text(
				"Easily Change keybinds to whatever button you'd like.\n"
			);
			ImGui::Separator();
			ImGui::Indent(5);

			ImGui::Dummy(ImVec2(0.0f, 5.0f));
			ImGui::PushID("KeybindInput");

			ImGui::PushItemWidth(long_width / 2);

			char keybindInput[128] = "";
			if (ImGui::InputTextWithHint("", "Keybind", keybindInput, IM_ARRAYSIZE(keybindInput), ImGuiInputTextFlags_CharsNoBlank)) {
				*keybindHolder = keybindInput;
			}
			ImGui::SameLine();
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Type out desired keybind.\n"
					"Use capital first letter, lowercase next letters, PascalCase for compound words. \n\n"
					"i.e. Home, End, NumPadFive, NumPadOne, LeftAlt. \n\n"
					"NumPad +, -, * are just Add, Subtract, Multiply. "
				);

			ImGui::PopItemWidth();
			ImGui::PopID();

			ImGui::Dummy(ImVec2(0.0f, 10.0f));
			ImGui::PushID("GuiButton");

			ImGui::Text("Open GUI");
			ImGui::SameLine(100);

			if (ImGui::Button("set"))
			{
				std::string unbind = "unbind " + *gui_keybind + " " + "ps_gui";
				std::string bind = "bind " + *keybindHolder + " " + "ps_gui";

				_globalCvarManager->executeCommand(unbind);
				_globalCvarManager->executeCommand(bind);
				setNewGUIKeybind(*keybindHolder);
				_globalCvarManager->executeCommand("writeconfig", true);
			}
			ImGui::PopID();

			ImGui::SameLine();
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Set keybind to open/close the GUI.\n"
					"Avoid F2 (Bakkesmod) and F6 (Bakkesmod Console."
				);
			ImGui::SameLine();
			ImGui::Text("Bound key: % s\n", *gui_keybind);

			ImGui::Dummy(ImVec2(0.0f, 10.0f));
			ImGui::PushID("ToggleButton");

			if (*plugin_keybind == "") {
				setNewPluginKeybind("Unset");
			}

			ImGui::Text("Toggle Plugin");
			ImGui::SameLine(100);
			if (ImGui::Button("set"))
			{
				_globalCvarManager->executeCommand("unbind " + *plugin_keybind + " change_ps_enabled");
				_globalCvarManager->executeCommand("bind " + *keybindHolder + " change_ps_enabled");
				_globalCvarManager->executeCommand("writeconfig", true);
				setNewPluginKeybind(*keybindHolder);
			}
			ImGui::SameLine();
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Set keybind to immediately enable/disable the plugin.\n"
					"Avoid F2 (Bakkesmod) and F6 (Bakkesmod Console)."
				);
			ImGui::SameLine();
			ImGui::Text("Bound key: % s\n", *plugin_keybind);
			ImGui::PopID();

			ImGui::Dummy(ImVec2(0.0f, 10.0f));

			ImGui::EndChild();
		}
		ImGui::EndTabItem();
	}
}

/// <summary> Renders main GUI settings. </summary>
void PremierSuite::renderSettingsTab()
{
	if (ImGui::BeginTabItem("Settings"))
	{
		ImGui::Spacing();
		ImGui::Text(
			"PremierSuite:\n"
			"Instant Access to Auto-Queue, Freeplay, and Custom Training."
		);

		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		ImGui::Separator();
		ImGui::Dummy(ImVec2(0.0f, 5.0f));

		//-----------------------------------------------------------------------------
		// Plugin Options
		//-----------------------------------------------------------------------------
		{
			ImGui::Text("Plugin Options");

			ImGui::Dummy(ImVec2(0.0f, 5.0f));
			ImGui::PushID("Toggler");
			if (ToggleButton("", &*enabled))
			{
				setEnablePlugin(enabled);
				_globalCvarManager->executeCommand("writeconfig", false);
			}

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Enable/Disable the plugin and all of its functionality.\n"
					"You can also bind this to a key in the Keybinds tab!.\n");

			ImGui::PopID();

			ImGui::Dummy(ImVec2(0.0f, 5.0f));

			if (!*enabled) {
				ImGui::PushID("Disabled");
				ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			}

			if (ImGui::Checkbox("Disable Plugin for Private Matches", &*disablePrivate)) {
				setDisablePrivate(disablePrivate);
				_globalCvarManager->executeCommand("writeconfig", false);
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("When this box is checked, the plugin will deactivate during private matches.");
		}

		ImGui::Dummy(ImVec2(0.0f, 10.0f));
		ImGui::Separator();
		ImGui::Dummy(ImVec2(0.0f, 5.0f));

		//-----------------------------------------------------------------------------
		// Auto-Queue Options
		//-----------------------------------------------------------------------------
		{
			ImGui::Text("Auto-Queue Options");
			ImGui::Dummy(ImVec2(0.0f, 5.0f));
			if (ImGui::Checkbox("Enable", &*enableQueue)) {
				setEnableQueue(enableQueue);
				_globalCvarManager->executeCommand("writeconfig", false);
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Automatically queue when an online match has ended.");
			ImGui::SameLine(150);

			ImGui::PushItemWidth(long_width);
			ImGui::PushID("queueDelayTime");
			if (ImGui::SliderFloat("", &*delayQueue, 0.0f, 10.0f, "Delay: %.1f s")) {
				setDelayQueue(delayQueue);
				_globalCvarManager->executeCommand("writeconfig", false);
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Set a delay for the auto-queue.");

			ImGui::PopID();
			ImGui::PopItemWidth();

			ImGui::PushID("disableQ");
			if (ImGui::Checkbox("Disable for Casual", &*disableQueueCasual)) {
				setDisableQueueCasual(disableQueueCasual);
				_globalCvarManager->executeCommand("writeconfig", false);
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Disable the automatic queue for casual games.");
			ImGui::PopID();
		}

		ImGui::Dummy(ImVec2(0.0f, 5.0f));
		ImGui::Separator();
		ImGui::Dummy(ImVec2(0.0f, 5.0f));

		//-----------------------------------------------------------------------------
		// Auto-Exit Options
		//-----------------------------------------------------------------------------
		{
			ImGui::Text("Auto-Exit Options");
			ImGui::SameLine();
			ImGui::TextDisabled("(?)");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Exit to desired location immediately, or with delay.\n"
					"Current options: Main-Menu, Freeplay, Custom Training Pack.\n"
					"Workshops, private game modes and specialty game modes are on the way.\n\n"
					"*If multiple exit-options are enabled, the uppmost most selection will be executed.");
			ImGui::Dummy(ImVec2(0.0f, 5.0f));

			/// Freeplay ----------------------------------------
			{
				if (ImGui::Checkbox("Freeplay", &*freeplayEnabled)) {
					setEnableFreeplay(freeplayEnabled);
					_globalCvarManager->executeCommand("writeconfig", false);
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Instant-exit to Freeplay.\n");
				ImGui::SameLine(150);

				std::string currMap = *freeplayMap;
				const char* currMapChr = currMap.c_str();
				int index = std::distance(freeplayMaps.begin(), std::find(freeplayMaps.begin(), freeplayMaps.end(), currMap));
				ImGui::PushItemWidth(long_width);
				//ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x * 0.9f);
				if (ImGui::SearchableCombo("##", &index, freeplayMaps, "no maps found", "type to search"))
				{
					setFreeplayMap(freeplayMaps[index]);
					ImGui::EndCombo();
				}
				ImGui::PopItemWidth();
			}
			ImGui::Dummy(ImVec2(0.0f, 5.0f));

			/// Custom Training ----------------------------------------
			{
				if (ImGui::Checkbox("Custom Training", &*customEnabled)) {
					setEnableCustomTraining(customEnabled);
					_globalCvarManager->executeCommand("writeconfig", false);
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Instant-exit to Custom Training.\n");
				ImGui::SameLine(150);
				ImGui::PushItemWidth(long_width);
				ImGui::PushID("InputCodeCustom");

				std::string str0 = std::string();
				std::string c = *customCode;
				ImGuiInputTextFlags ctflags = ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_EnterReturnsTrue;
				bool ctInput = ImGui::InputTextWithHint("", c.c_str(), &str0, ctflags);
				if (ctInput) {
					setCustomTrainingCode(str0);
					str0 = std::string();
					_globalCvarManager->executeCommand("writeconfig", false);
					ImGui::SetItemDefaultFocus();
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Training pack code here, ENTER to apply.\n"
						"The default is A0FE-F860-967D-E628: GC Balls to Master.\n"
						"This is one I made.  :) \n\n"
						"...its hard"
					);
				ImGui::PopID();
			}

			ImGui::Dummy(ImVec2(0.0f, 5.0f));

			/// Workshop Maps ----------------------------------------
			{
				if (!*isOnSteam) {
					ImGui::PushID("NotSteam");
					ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
				}

				ImGui::PushID("WorshopCheckbox");
				if (ImGui::Checkbox("Workshop", &*workshopEnabled)) {
					setEnableWorkshop(workshopEnabled);
					_globalCvarManager->executeCommand("writeconfig", false);
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Instantly load into a workshop map.\n");
				ImGui::SameLine(150);

				std::string currentWorkshopMap = *workshopMap;
				const char* workshop_selection = currentWorkshopMap.c_str();
				int workshop_index = std::distance(workshopMapNames.begin(), std::find(workshopMapNames.begin(), workshopMapNames.end(), currentWorkshopMap));
				ImGui::PushItemWidth(long_width);

				if (ImGui::SearchableCombo("##", &workshop_index, workshopMapNames, "no maps selected", "type to search"))
				{
					setWorkshopMap(workshopMapNames[workshop_index]);
					setEnableWorkshop(workshopEnabled);
					ImGui::EndCombo();
				}
				ImGui::PopItemWidth();
				ImGui::PopID();

				if (!*isOnSteam) {

					ImGui::SameLine(150);
					ImGui::PopItemFlag();
					ImGui::PopStyleVar();
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Workshops incoming for epic.\n");
					ImGui::PopID();
				}
			}

			ImGui::Dummy(ImVec2(0.0f, 5.0f));

			/// Main Menu ----------------------------------------
			{
				if (ImGui::Checkbox("Main Menu", &*exitEnabled)) {
					setEnableExit(exitEnabled);
					_globalCvarManager->executeCommand("writeconfig", false);
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Instant-exit to Main-Menu.\n");
			}

			ImGui::Dummy(ImVec2(0.0f, 5.0f));

			/// Disable Exit for Casual
			{
				if (ImGui::Checkbox("Disable for Casual", &*disableExitCasual)) {
					setDisableExitCasual(disableExitCasual);
					_globalCvarManager->executeCommand("writeconfig", false);
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Disable the all automatic-exit settings for casual matches..");

				ImGui::SameLine(150);
				ImGui::PushItemWidth(long_width);
				ImGui::PushID("exitDelayTime");
				if (ImGui::SliderFloat("", &*delayExit, 0.0f, 10.0f, "Delay: %.1f s")) {
					setDelayExit(delayExit);
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Set a delay for auto-exit options (options are below).");
				ImGui::PopItemWidth();
				ImGui::PopID();
			}
			ImGui::Dummy(ImVec2(0.0f, 5.0f));

			if (!*enabled) {
				ImGui::PopItemFlag();
				ImGui::PopStyleVar();
				ImGui::PopID();
			}
		}
		ImGui::EndTabItem();
	}
}

/// <summary> Renders about window. </summary>
void PremierSuite::renderAboutWindow(bool* open)
{
	if (!ImGui::Begin("About PremierSuite", open, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}
	ImGui::Separator();
	ImGui::Text("By Flynn OConnell with help Xenobyte and many BakkesMod Plugin Developers.");
	ImGui::Text("This plugin was started by and an extension of instantsuite @iamxenobyte: https://xenobyte.dev/ ");
	ImGui::Text("PremierSuite is licensed under the MIT License, see LICENSE for more information.");
	ImGui::Text("Source code for this plugin is located at: https://github.com/NeuroPyPy/PremierSuite");
	ImGui::End();
}

bool PremierSuite::ToggleButton(const char* str_id, bool* v)
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

bool PremierSuite::Button(const char* label, bool enabled = true, const ImVec2& size = ImVec2(0, 0)) {
	if (enabled) {
		return ImGui::Button(label, size);
	}
	else {
		ImU32 const disabled_fg = IM_COL32_BLACK;
		ImU32 const disabled_bg = IM_COL32(64, 64, 64, 255);

		ImGui::PushStyleColor(ImGuiCol_Text, disabled_fg);
		ImGui::PushStyleColor(ImGuiCol_Button, disabled_bg);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, disabled_bg);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, disabled_bg);
		ImGui::Button(label, size);
		ImGui::PopStyleColor(4);

		return false;
	}