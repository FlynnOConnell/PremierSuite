#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"
#include "version.h"
#include "utils/parser.h"
#include "utils/io.h"
#include <filesystem>
#include "parser.h"
//#include "IMGUI/imgui_searchablecombo.h" // Likely need to modify this to allow searching workshops/custom maps


constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);

// Default Keybinds
constexpr const char* DEFAULT_GUI_KEYBIND = "F3";

extern std::filesystem::path BakkesModConfigFolder;
extern std::filesystem::path PremierSuiteDataFolder;
extern std::filesystem::path RocketLeagueExecutableFolder;

#define BINDS_FILE_PATH        (BakkesModConfigFolder / "binds.cfg")
#define CONFIG_FILE_PATH       (BakkesModConfigFolder / "config.cfg")
#define COOKED_PC_CONSOLE_PATH (RocketLeagueExecutableFolder / "../../TAGame/CookedPCConsole")
#define CUSTOM_MAPS_PATH       (COOKED_PC_CONSOLE_PATH / "mods")
#define COPIED_MAPS_PATH       (COOKED_PC_CONSOLE_PATH / "PremierSuite")
#define WORKSHOP_MAPS_PATH     (RocketLeagueExecutableFolder / "../../../../workshop/content/252950") //Generally C:\Program Files (x86)\Steam\steamapps\workshop\content\252950


class PremierSuite : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginWindow
{
	std::shared_ptr<bool> enabled;
	virtual void onLoad();
	virtual void onUnload();

public:
	// Help return lowercase if need be for epic games workshop integration 
	static std::string toLower(std::string str, bool changeInline = false);

private:
	// General Declaration
	void pluginEnabledChanged();
	void launchCustomTraining(ServerWrapper caller, void* params, std::string eventName);
	void launchTraining(ServerWrapper caller, void* params, std::string eventName);
	void launchWorkshop(ServerWrapper caller, void* params, std::string eventName);
	void delayedCustomTraining();
	void delayedTraining();
	void delayedWorkshop();
	void queue(ServerWrapper caller, void* params, std::string eventName);
	void exitGame(ServerWrapper caller, void* params, std::string eventName);
	void delayedQueue();
	void delayedExit();
	void removeOldPlugin();
	void hookMatchEnded();
	void unhookMatchEnded();
	void logHookType(const char* const hookType) const;
	void onMatchEnd(ServerWrapper server, void* params, std::string eventName);

	void setDisableCasualQueue(bool newDisCasualQueue);
	void setDisablePrivate(bool newDisPrivate);
	void setDisableCasual(bool newDisCasual);

	void setDelayedQueue(float newQueueDelayTime);
	void setDelay(float newDelay);

	void setPluginEnabled(bool newPluginEnabled);
	void setQueueEnabled(bool newQueueEnabled);
	void setTrainingEnabled(bool newTrainingEnabled);
	void setCustomTrainingEnabled(bool newCustomTrainingEnabled);
	void setCustomTrainingCode(std::string newCustomTrainingCode);
	void setWorkshopEnabled(bool newWorkshopEnabled);
	void setExitEnabled(bool newExitEnabled);



	int rank_nb = 23;

	SteamID mySteamID = { 0 };

	// Cvar Declaration
	static constexpr const char* matchEndedEvent = "Function TAGame.GameEvent_Soccar_TA.EventMatchEnded";
	static constexpr const char* enabledCvarName = "is_enablePlugin";
	static constexpr const char* trainingCvarName = "is_enableTraining";
	static constexpr const char* ctrainingCvarName = "is_enableCTraining";
	static constexpr const char* wtrainingCvarName = "is_enableWorkshop";
	static constexpr const char* queueCvarName = "is_enableQueue";
	static constexpr const char* exitCvarName = "is_enableExit";

	static constexpr const char* keybindCvarName = "is_gui_keybind";
	static constexpr const char* keyholderCvarName = "key_holder";

	static constexpr const char* trainingMapCvarName = "is_trainingMap";
	static constexpr const char* customtrainingCvarName = "is_ctrainingMap";
	static constexpr const char* workshopCvarName = "is_workshopMap";

	static constexpr const char* DelayCvarName = "is_Delay";
	static constexpr const char* qDelayCvarName = "is_QDelay";

	static constexpr const char* disableCasualQCvarName = "is_QBypassCasual";
	static constexpr const char* disableCasualCvarName = "is_bypassCasual";
	static constexpr const char* disablePrivateCvarName = "is_bypassPrivate";

	static constexpr const char* newPackNameCvar = "newPackNameCvar";
	static constexpr const char* newPackCodeCvar = "newPackCodeCvar";
	static constexpr const char* newPackAuthorCvar = "newPackAuthorCvar";



	// File Helpers
	std::shared_ptr<std::string> workshopMapDirPath;
	std::shared_ptr<std::string> customMapDirPath;
	std::vector<std::filesystem::path> getWorkshopMaps(const std::filesystem::path& workshopPath,
		const std::vector<std::string>& extensions = { ".upk", ".udk" },
		const std::string& preferredExtension = ".udk");
	static std::vector<std::filesystem::path> IterateDirectory(const std::filesystem::path& directory, const std::vector<std::string>& extensions, int depth = 0, int maxDepth = 3);
	static std::vector<std::filesystem::path> GetFilesFromDir(const std::filesystem::path& directory, int numExtension, ...);

	// Formatting workshop/freeplay map returns 
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
	//std::wstring GetPlayerNickname(uint64_t uniqueId);							
	void changeGuiKeybind();
	void unbindKeybind();
	void changePluginEnabledKeybind();
	void quickPluginEnabled();

	// Window settings
	bool isWindowOpen = false;
	bool isMinimized = false;
	std::string menuTitle = "PremierSuite";
public:
	virtual void Render();
	virtual std::string GetMenuName();
	virtual std::string GetMenuTitle();
	virtual void SetImGuiContext(uintptr_t ctx);
	virtual bool ShouldBlockInput();
	virtual bool IsActiveOverlay();
	virtual void OnOpen();
	virtual void OnClose();

private:
	void renderInstantSettingsTab();
	void renderTrainingPacksTab();
	void renderKeybindsTab();
	bool hooked = false;
	bool enableCustomMaps = false;
	bool enableWorkshopMaps = false;
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


	std::map<std::string, PackInfo> allPacks;

	bool renderCustomMapsSelection(std::map<std::filesystem::path, std::string>& customMaps,
		std::filesystem::path&, bool& refreshCustomMaps,
		bool includeWorkshopMaps = true, bool includeCustomMaps = true);

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
