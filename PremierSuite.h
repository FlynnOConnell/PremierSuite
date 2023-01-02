#pragma once

#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "GuiBase.h"
#include "version.h"

constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

extern std::filesystem::path BakkesModConfigFolder;
extern std::filesystem::path PremierSuiteDataFolder;
extern std::filesystem::path RocketLeagueExecutableFolder;

#define CONFIG_FILE_PATH		(BakkesModConfigFolder / "config.cfg")
#define BINDS_FILE_PATH			(BakkesModConfigFolder / "binds.cfg")
#define PLUGINS_FILE_PATH		(BakkesModConfigFolder / "plugins.cfg")
#define FONTS_PATH			    (CORE_DATA_PATH / "fonts" )
#define STYLES_PATH			    (CORE_DATA_PATH / "styles" )
#define COOKED_PC_CONSOLE_PATH  (RocketLeagueExecutableFolder / "../../TAGame/CookedPCConsole")
#define CUSTOM_MAPS_PATH        (COOKED_PC_CONSOLE_PATH / "mods")
#define COPIED_MAPS_PATH        (COOKED_PC_CONSOLE_PATH / "PremierSuite")
#define WORKSHOP_MAPS_PATH      (RocketLeagueExecutableFolder / "../../../../workshop/content/252950") //Generally C:\Program Files (x86)\Steam\steamapps\workshop\content\252950

//#define DEBUG

class PremierSuite : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginWindow
{
public:
	std::shared_ptr<bool> enabled;
	std::shared_ptr<bool> autoGG;
	std::shared_ptr<bool> freeplayEnabled;
	std::shared_ptr<bool> customEnabled;
	std::shared_ptr<bool> exitEnabled;
	std::shared_ptr<bool> workshopEnabled;

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
	std::shared_ptr<std::string> freeplayMap; //TODO combo with maps
	std::shared_ptr<std::string> workshopMapDirPath;
	std::shared_ptr<std::string> customMapDirPath;
	
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
	void renderMenu();
	void renderAboutWindow(bool* p_open);
	void loadFont();

	ImFont* myFont;

	virtual void onLoad();
	static std::string toLower(std::string str, bool changeInline = false);
	
	void callbackSetDelay(ServerWrapper server, void* params, std::string eventName, void* callback);


private:

	bool isWindowOpen_ = false;
	bool isMinimized = false;
	std::string menuTitle_ = "PremierSuite " + std::string(std::string_view(plugin_version).substr(0, std::string_view(plugin_version).rfind('.')));

	std::vector<std::string> parseCfg(const std::string searchString, bool log = false);

	//bool isRanked(ServerWrapper server);
	void launchCustomTraining(ServerWrapper caller, void* params, std::string eventName);
	void launchFreeplay(ServerWrapper caller, void* params, std::string eventName);
	void launchWorkshop(ServerWrapper caller, void* params, std::string eventName);
	void delayedCustomTraining();
	void delayedFreeplay();
	void delayedWorkshop();
	void queue(ServerWrapper caller, void* params, std::string eventName);
	void exitGame(ServerWrapper caller, void* params, std::string eventName);
	void delayedQueue();
	void delayedExit();
	void hookMatchEnded();
	void unhookMatchEnded();
	void logHookType(const char* const hookType) const;
	void onMatchEnd(ServerWrapper server, void* params, std::string eventName);
	void checkConflicts();

public:

	//-----------------------------------------------------------------------------
	//--- Functions needed in GuiBase.cpp for GUI rendering
	//-----------------------------------------------------------------------------

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

	bool isRanked(ServerWrapper server);
	bool isPrivate(ServerWrapper server);
	bool isTournament(ServerWrapper server);
	bool isStandard(ServerWrapper server);

	static constexpr const char* matchEndedEvent = "Function TAGame.GameEvent_Soccar_TA.EventMatchEnded";

	std::vector<std::filesystem::path> getWorkshopMaps(const std::filesystem::path& workshopPath,
		const std::vector<std::string>& extensions = { ".upk", ".udk" },
		const std::string& preferredExtension = ".udk");
	static std::vector<std::filesystem::path> IterateDirectory(const std::filesystem::path& directory, const std::vector<std::string>& extensions, int depth = 0, int maxDepth = 3);
	static std::vector<std::filesystem::path> GetFilesFromDir(const std::filesystem::path& directory, int numExtension, ...);

	struct WorkshopMap {
		std::string Title;
		uint64_t owner = 0;
	};
	struct FreeplayMap {
		std::string Title;
		uint64_t owner = 0;
	};

	std::vector<uint64_t> publishedFileID;
	static bool HasExtension(const std::string& fileExtension, const std::vector<std::string>& extensions);

	std::unordered_map<uint64_t, WorkshopMap> subscribedWorkshopMaps;
	std::unordered_map<uint64_t, FreeplayMap> freeplayMaps;

private:

	void handleKeybindCvar();
	bool hooked = false;
	bool enableCustomMaps = false;
	bool refreshCustomMapPaths = true;
	int currentFreeplayMap = 0;

	// Maps customMapPaths key or map path
	std::string currentMap;
	std::string currentMapFile;
	std::vector<std::filesystem::path> otherMapPaths;
	std::vector<std::filesystem::path> presetPaths;

	// Maps internal name to display name
	std::map<std::string, std::string> maps;
	// Maps path to display name 
	std::map<std::filesystem::path, std::string> customMapPaths;

	struct PackInfo {
		std::string packName;
		std::string packCode;
		std::string packAuthor;
		bool loaded = false;
	};

	const std::map<std::string, std::string> MAPS{
		{ "ARC_P",                   "ARCtagon" },
		{ "CHN_Stadium_Day_P",       "Forbidden Temple (Day)" },
		{ "CHN_Stadium_P",           "Forbidden Temple" },
		{ "EuroStadium_Night_P",     "Mannfield (Night)" },
		{ "EuroStadium_P",           "Mannfield" },
		{ "EuroStadium_Rainy_P",     "Mannfield (Stormy)" },
		{ "Farm_Night_P",            "Farmstead (Night)" },
		{ "Haunted_TrainStation_P",  "Urban Central (Haunted)" },
		{ "HoopsStadium_P",          "Dunk House" },
		{ "Labs_Basin_P",            "Basin" },
		{ "Labs_CirclePillars_P",    "Pillars" },
		{ "Labs_Corridor_P",         "Corridor" },
		{ "Labs_Cosmic_P",           "Cosmic (Old)" },
		{ "Labs_Cosmic_V4_P",        "Cosmic (New)" },
		{ "Labs_DoubleGoal_P",       "Double Goal (Old)" },
		{ "Labs_DoubleGoal_V2_P",    "Double Goal (New)" },
		{ "Labs_Galleon_Mast_P",     "Galleon Retro" },
		{ "Labs_Galleon_P",          "Galleon" },
		{ "Labs_Octagon_02_P",       "Octagon (New)" },
		{ "Labs_Octagon_P",          "Octagon (Old)" },
		{ "Labs_Underpass_P",        "Underpass" },
		{ "Labs_Utopia_P",           "Utopia Retro" },
		{ "NeoTokyo_P",              "Tokyo Underpass" },
		{ "NeoTokyo_Standard_P",     "Neo Tokyo" },
		{ "Park_Night_P",            "Beckwith Park (Midnight)" },
		{ "Park_P",                  "Beckwith Park" },
		{ "Park_Rainy_P",            "Beckwith Park (Stormy)" },
		{ "ShatterShot_P",           "Core 707" },
		{ "Stadium_Foggy_P",         "DFH Stadium (Stormy)" },
		{ "Stadium_P",               "DFH Stadium" },
		{ "Stadium_Race_Day_p",      "DFH Stadium (Circuit)" },
		{ "Stadium_Winter_P",        "DFH Stadium (Snowy)" },
		{ "TrainStation_Dawn_P",     "Urban Central (Dawn)" },
		{ "TrainStation_Night_P",    "Urban Central (Night)" },
		{ "TrainStation_P",          "Urban Central" },
		{ "Underwater_P",            "AquaDome" },
		{ "UtopiaStadium_Dusk_P",    "Utopia Coliseum (Dusk)" },
		{ "UtopiaStadium_P",         "Utopia Coliseum" },
		{ "UtopiaStadium_Snow_P",    "Utopia Coliseum (Snowy)" },
		{ "Wasteland_Night_P",       "Badlands (Night)" },
		{ "Wasteland_P",             "Badlands" },
		{ "arc_standard_p",          "Starbase ARC" },
		{ "bb_p",                    "Champions Field (NFL)" },
		{ "beach_P",                 "Salty Shores" },
		{ "beach_night_p",           "Salty Shores (Night)" },
		{ "cs_day_p",                "Champions Field (Day)" },
		{ "cs_hw_p",                 "Rivals Arena" },
		{ "cs_p",                    "Champions Field" },
		{ "eurostadium_snownight_p", "Mannfield (Snowy)" },
		{ "farm_p",                  "Farmstead" },
		{ "music_p",                 "Neon Fields" },
		{ "stadium_day_p",           "DFH Stadium (Day)" },
		{ "throwbackhockey_p",       "Throwback Stadium (Snowy)" },
		{ "throwbackstadium_P",      "Throwback Stadium" },
		{ "wasteland_Night_S_P",     "Wasteland (Night)" },
		{ "wasteland_s_p",           "Wasteland" }
	};

	const std::vector<std::string> beforeSelect = { "Select Workshop Map" };
	const std::vector<std::string> beforeFreeplaySelect = { "Select Freeplay Map" };



};
