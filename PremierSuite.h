#pragma once

#include <ranges>
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "bakkesmod/wrappers/GuiManagerWrapper.h"

#include "GuiBase.h"
#include "version.h"

// Get-Content (-join($env:APPDATA, "\bakkesmod\bakkesmod\bakkesmod.log")) -wait -tail 1 | select-string -pattern "PremierSuite"

constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

extern std::filesystem::path BakkesModConfigFolder;
extern std::filesystem::path RocketLeagueExecutableFolder;

#define CONFIG_FILE_PATH		(BakkesModConfigFolder / "config.cfg")
#define BINDS_FILE_PATH			(BakkesModConfigFolder / "binds.cfg")
#define PLUGINS_FILE_PATH		(BakkesModConfigFolder / "plugins.cfg")
#define PREMIERSUITE_FILE_PATH	(BakkesModConfigFolder / "PremierSuite.cfg")
#define WORKSHOP_MAPS_PATH      (RocketLeagueExecutableFolder / "../../../../workshop/content/252950") //Generally C:\Program Files (x86)\Steam\steamapps\workshop\content\252950

//#define DEBUG

class PremierSuite : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginWindow
{
public:
	std::shared_ptr<bool> isOnSteam;
	std::shared_ptr<bool> enabled;
	std::shared_ptr<bool> autoGG;
	std::shared_ptr<bool> freeplayEnabled;
	std::shared_ptr<bool> customEnabled;
	std::shared_ptr<bool> exitEnabled;
	std::shared_ptr<bool> workshopEnabled;
	std::shared_ptr<bool> customMapEnabled;

	std::shared_ptr<bool> disablePrivate;
	std::shared_ptr<bool> enableQueue;
	std::shared_ptr<bool> disableQueueCasual;
	std::shared_ptr<bool> disableExitCasual;

	std::shared_ptr<float> autoGGDelay;
	std::shared_ptr<float> delayQueue;
	std::shared_ptr<float> delayExit;

	std::shared_ptr<std::string> gui_keybind;
	std::shared_ptr<std::string> plugin_keybind;
	std::shared_ptr<std::string> customCode;
	std::shared_ptr<std::string> workshopMap;
	std::shared_ptr<std::string> freeplayMap;
	std::shared_ptr<std::string> workshopMapDirPath;

    std::vector<std::string> freeplayMaps;

	std::string getClient();
	std::string rlClient = getClient();
	
	std::string GetMenuName() override;
	std::string GetMenuTitle() override;
	void SetImGuiContext(uintptr_t ctx) override;
	bool ShouldBlockInput() override;
	bool IsActiveOverlay() override;
	void OnOpen() override;
	void OnClose() override;

	void Render();
	void renderSettingsTab();
	void renderKeybindsTab();

	void CheckFont();
	void renderMenu();
	void renderAboutWindow(bool* p_open);
	bool ToggleButton(const char* str_id, bool* v);

	virtual void onLoad();

	ImFont* myRoboFont;

private:

	bool isWindowOpen_ = false;
	bool isMinimized = false;
	std::string menuTitle_ = "PremierSuite (" + rlClient + ") " + "v." + std::string(std::string_view(plugin_version).substr(0, std::string_view(plugin_version).rfind('.')));
	void checkConflicts();
	
	void onMatchEnd(ServerWrapper server, void* params, std::string eventName);
	void callbackSetDelay(ServerWrapper server, void* params, std::string eventName, std::function<void()> callback, bool queue);

	void executeWorkshop();
	void executeCustomTraining();
	void executeFreeplay();
	void executeQueue();
	void executeMainMenu();

	void registerNotifiers();
	void registerCvars();
	void hookMatchEnded();
	void unhookMatchEnded();
	void logHookType(const char* const hookType) const;
	static constexpr const char* matchEndedEvent = "Function TAGame.GameEvent_Soccar_TA.EventMatchEnded";

	std::string getInGameMap();
	void handleKeybindCvar();
	void logVector(std::vector<std::string> inputVec);
	void logVector(std::vector<std::filesystem::path> inputVec);
	std::string btos(bool x);

public:

	void quickPluginEnabled();

	void setEnablePlugin(std::shared_ptr<bool> newBool);
	void setEnableCustomTraining(std::shared_ptr<bool> newBool);
	void setEnableWorkshop(std::shared_ptr<bool> newBool);
	void setEnableFreeplay(std::shared_ptr<bool> newBool);
	void setEnableExit(std::shared_ptr<bool> newBool);
	void setEnableQueue(std::shared_ptr<bool> newBool);

	void setDisableQueueCasual(std::shared_ptr<bool> newBool);
	void setDisableExitCasual(std::shared_ptr<bool> newBool);
	void setDisablePrivate(std::shared_ptr<bool> newBool);

	void setDelayQueue(std::shared_ptr<float> newFloat);
	void setDelayExit(std::shared_ptr<float> newFloat);

	void setCustomTrainingCode(char newStr);
	void setCustomTrainingCode(std::string newStr);
	void setNewGUIKeybind(std::string newKeybind);
	void setNewPluginKeybind(std::string newKeybind);
	void setFreeplayMap(std::string newMap);
	void setWorkshopMap(std::string newMap);

	bool isRanked(ServerWrapper server);
	bool isPrivate(ServerWrapper server);
	bool isTournament(ServerWrapper server);
	bool isStandard(ServerWrapper server);
	
	const char* getCurrentFreeplayMap();

	int* getIndex(std::vector<std::string> v, std::string str);
	[[nodiscard]] std::vector<std::string> ValsToVec(std::map<std::string, std::string> map) const;
	[[nodiscard]] std::vector<std::string> KeysToVec(std::map<std::string, std::string> map) const;

	std::string GetKeyFromValue(std::map<std::string, std::string>, std::string val);

	std::vector<std::string> parseCfg(std::filesystem::path filePath, const std::string searchString, bool log);


	std::vector<std::filesystem::path> getWorkshopMaps(const std::filesystem::path& workshopPath);
	static std::vector<std::filesystem::path> IterateDirectory(const std::filesystem::path& directory, const std::vector<std::string>& extensions, int depth = 0, int maxDepth = 3);
	static std::vector<std::filesystem::path> GetFilesFromDir(const std::filesystem::path& directory, int numExtension, ...);
	static bool HasExtension(const std::string& fileExtension, const std::vector<std::string>& extensions);
	static std::string toLower(std::string str, bool changeInline = false);

private:

	bool hooked = false;
	bool enableCustomMaps = false;
	bool refreshCustomMapPaths = true;
	std::map<std::string, std::string> maps;

	[[nodiscard]] void set_udk_files(const std::filesystem::path& root_dir);

	std::vector<std::string> workshopMapNames;
	std::map<std::string, std::string> WorkshopMaps;

	const std::map<std::string, std::string> FreeplayMaps{
		{"random",					 "random"},
		{ "CHN_Stadium_Day_P",       "Forbidden Temple (Day)" },
		{ "CHN_Stadium_P",           "Forbidden Temple" },
		{ "EuroStadium_Night_P",     "Mannfield (Night)" },
		{ "EuroStadium_P",           "Mannfield" },
		{ "EuroStadium_Rainy_P",     "Mannfield (Stormy)" },
		{ "Farm_Night_P",            "Farmstead (Night)" },
		{ "HoopsStadium_P",          "Dunk House (Hoops)" },
		{ "hoopsStreet_p",			 "The Block (Hoops"},
		{ "NeoTokyo_Standard_P",     "Neo Tokyo" },
		{ "Park_Night_P",            "Beckwith Park (Midnight)" },
		{ "Park_P",                  "Beckwith Park" },
		{ "Park_Rainy_P",            "Beckwith Park (Stormy)" },
		{ "ShatterShot_P",           "Core 707 (DropShot)" },
		{ "Stadium_Foggy_P",         "DFH Stadium (Stormy)" },
		{ "Stadium_P",               "DFH Stadium" },
		{ "Stadium_Race_Day_p",      "DFH Stadium (Circuit)" },
		{ "Stadium_Winter_P",        "DFH Stadium (Snowy)" },
		{ "street_p",				 "Sovereign Heights" },
		{ "TrainStation_Dawn_P",     "Urban Central (Dawn)" },
		{ "TrainStation_Night_P",    "Urban Central (Night)" },
		{ "TrainStation_P",          "Urban Central" },
		{ "Underwater_P",            "AquaDome" },
		{ "UtopiaStadium_Dusk_P",    "Utopia Coliseum (Dusk)" },
		{ "UtopiaStadium_P",         "Utopia Coliseum" },
		{ "UtopiaStadium_Snow_P",    "Utopia Coliseum (Snowy)" },
		{ "beach_night_p",           "Salty Shores (Night)" },
		{ "cs_day_p",                "Champions Field (Day)" },
		{ "cs_hw_p",                 "Rivals Arena" },
		{ "cs_p",                    "Champions Field" },
		{ "farm_p",                  "Farmstead" },
		{ "music_p",                 "Neon Fields" },
		{ "stadium_day_p",           "DFH Stadium (Day)" },
		{ "throwbackstadium_P",      "Throwback Stadium" },
		{ "wasteland_Night_S_P",     "Wasteland (Night)" },
	};

};

