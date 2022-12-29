#include "pch.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <Windows.h>
#include <filesystem>
#include "PremierSuite.h"
#include <json/json.h>
#include "PersistentStorage.h"

#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

BAKKESMOD_PLUGIN(PremierSuite, "Premier Suite", plugin_version, PLUGINTYPE_FREEPLAY | PLUGINTYPE_CUSTOM_TRAINING)

std::filesystem::path BakkesModConfigFolder;
std::filesystem::path PremierSuiteDataFolder;
std::filesystem::path RocketLeagueExecutableFolder;
std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

std::shared_ptr<int> numKeybinds;

enum Mode
{
	CasualDuel = 1,
	CasualDoubles = 2,
	CasualStandard = 3,
	CasualChaos = 4,
	Private = 6,
	RankedDuel = 10,
	RankedDoubles = 11,
	RankedSoloStandard = 12,
	RankedStandard = 13,
	Tournament = 22,
	RankedHoops = 27,
	RankedRumble = 28,
	RankedDropshot = 29,
	RankedSnowday = 30,
};

/// <summary>Debug type-helper. Returns the type of the parameter passed.</summary>
/// <typeparam name="T">Parameter to check.</typeparam>
/// <returns>Type of the parameter</returns>
template <typename T>
constexpr auto type_name() {
	std::string_view name, prefix, suffix;
#ifdef __clang__
	name = __PRETTY_FUNCTION__;
	prefix = "auto type_name() [T = ";
	suffix = "]";
#elif defined(__GNUC__)
	name = __PRETTY_FUNCTION__;
	prefix = "constexpr auto type_name() [with T = ";
	suffix = "]";
#elif defined(_MSC_VER)
	name = __FUNCSIG__;
	prefix = "auto __cdecl type_name<";
	suffix = ">(void)";
#endif
	name.remove_prefix(prefix.size());
	name.remove_suffix(suffix.size());
	return name;
}

//-----------------------------------------------------------------------------
// File Helper Functions
//-----------------------------------------------------------------------------

std::vector<std::string> split(const std::string& s, char delim) {
	std::vector<std::string> elems;
	std::stringstream ss(s);
	std::string item;
	while (getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

/// <summary>Returns the lowercased string from the given string.</summary>
/// <param name="str">String to change</param>
/// <param name="changeInline">Whether to change the string inline</param>
/// <returns>Returns the lowercased string from the given string</returns>
std::string PremierSuite::toLower(std::string str, bool changeInline)
{
	std::string str_cpy = str;
	std::transform(str_cpy.begin(), str_cpy.end(), str_cpy.begin(),
		[](unsigned char c) { return (unsigned char)std::tolower(c); });

	if (changeInline) {
		str = str_cpy;
	}

	return str_cpy;
}

/// <summary>Checks if the given file extension is in the list of extensions.</summary>
/// <param name="fileExtension">File extension</param>
/// <param name="extensions">List of file extensions</param>
/// <returns>Bool with if the file extension is in the list of extensions</returns>
bool PremierSuite::HasExtension(const std::string& fileExtension, const std::vector<std::string>& extensions)
{
	// Filter out unwanted file extensions.
	return std::any_of(extensions.begin(), extensions.end(), [&](const std::string& extension) {
		return fileExtension == extension;
		});
}

/// <summary>Recursively gets files from a certain directory.</summary>
/// <remarks>These files can be filtered by if they end with certain file extensions.</remarks>
/// <param name="directory">Path to the directory to get the files from</param>
/// <param name="extensions">List of file extensions to filter by</param>
/// <param name="depth">Current folder depth</param>
/// <param name="maxDepth">Max folder depth to iterate through</param>
/// <returns>The files from a certain directory</returns>
std::vector<std::filesystem::path> PremierSuite::IterateDirectory(const std::filesystem::path& directory,
	const std::vector<std::string>& extensions,
	const int depth, const int maxDepth)
{
	if (depth > maxDepth) {
		return std::vector<std::filesystem::path>();
	}

	std::vector<std::filesystem::path> files;
	for (const std::filesystem::directory_entry& file : std::filesystem::directory_iterator(directory)) {
		const std::filesystem::path& filePath = file.path();
		if (file.is_directory()) {
			std::vector<std::filesystem::path> directoryFiles = IterateDirectory(
				filePath, extensions, depth + 1, maxDepth);
			// Remove if directory is empty.
			if (!directoryFiles.empty()) {
				files.insert(files.end(), directoryFiles.begin(), directoryFiles.end());
			}
		}
		else if (HasExtension(filePath.extension().string(), extensions)) {
			files.push_back(filePath);
		}
	}

	return files;
}

/// <summary>Gets files from a certain directory.</summary>
/// <remarks>These files can be filtered by if they end with certain file extensions.</remarks>
/// <param name="directory">Path to the directory to get the files from</param>
/// <param name="numExtension">Number if filters to filter the files by</param>
/// <param name="...">Multiple file extensions to filter by</param>
/// <returns>The files from a certain directory</returns>
std::vector<std::filesystem::path> PremierSuite::GetFilesFromDir(const std::filesystem::path& directory,
	int numExtension, ...)
{
	if (!exists(directory)) {
		return std::vector<std::filesystem::path>();
	}

	va_list extensions;
	std::vector<std::string> fileExtensions;

	va_start(extensions, numExtension);
	for (int i = 0; i < numExtension; i++) {
		fileExtensions.emplace_back(va_arg(extensions, const char*));
	}
	va_end(extensions);

	return IterateDirectory(directory, fileExtensions, 0, 1);
}

/// <summary>Gets workshop maps from the given directory.</summary>
/// <param name="workshopPath">Path to the workshop directory to get the maps from</param>
/// <param name="extensions">List of map extensions to filter by</param>
/// <param name="preferredExtension">Map extension to prefer when multiple files are found</param>
/// <returns>The workshop maps from the given directory</returns>
std::vector<std::filesystem::path> PremierSuite::getWorkshopMaps(const std::filesystem::path& workshopPath,
	const std::vector<std::string>& extensions,
	const std::string& preferredExtension)
{
	if (!exists(workshopPath)) {
		return std::vector<std::filesystem::path>();
	}

	// Make sure we don't request workshop map names every tick.
	const bool shouldRequestWorkshopMapNames = publishedFileID.empty();
	std::vector<std::filesystem::path> files = IterateDirectory(workshopPath, extensions, 0, 1);
	std::filesystem::path bestPath;
	std::vector<std::filesystem::path> workshopMaps;
	for (const std::filesystem::path& file : files) {
		if (file.parent_path() != bestPath.parent_path()) {
			if (!bestPath.empty()) {
				const uint64_t workshopMapId = std::strtoull(bestPath.parent_path().stem().string().c_str(), nullptr,
					10);
				if (shouldRequestWorkshopMapNames && subscribedWorkshopMaps.find(workshopMapId) == subscribedWorkshopMaps.end()) {
					publishedFileID.push_back(workshopMapId);
				}
				workshopMaps.push_back(bestPath);
			}
			bestPath = file;
		}
		else if (bestPath.extension() != preferredExtension && file.extension() == preferredExtension) {
			bestPath = file;
		}
	}

	if (!bestPath.empty()) {
		const uint64_t workshopMapId = std::strtoull(bestPath.parent_path().stem().string().c_str(), nullptr, 10);
		if (shouldRequestWorkshopMapNames && subscribedWorkshopMaps.find(workshopMapId) == subscribedWorkshopMaps.end()) {
			publishedFileID.push_back(workshopMapId);
		}
		workshopMaps.push_back(bestPath);
	}


	return workshopMaps;
}

//-----------------------------------------------------------------------------
//--- Main Plugin Functions
//-----------------------------------------------------------------------------

void PremierSuite::setPluginEnabled(bool newPluginEnabled)
{
	cvarManager->getCvar("ps_enablePlugin").setValue(newPluginEnabled);
	cvarManager->log("ps. Plugin Enabled Cvar has been set");
}

void PremierSuite::pluginEnabledChanged()
{
	CVarWrapper enabledCvar = cvarManager->getCvar("ps_enablePlugin");
	bool enabled = enabledCvar.getBoolValue();
	if (enabled)
	{
		if (!hooked)
		{
			hookMatchEnded();
			cvarManager->log("ps. Hooked match ended (PluginEnabledChanged Function)");
		}
	}
	else
	{
		if (hooked)
		{
			unhookMatchEnded();
			cvarManager->log("Unhooked match ended (PluginEnabledChanged Function");
		}
	}
}

void PremierSuite::setDelay(float newDelay)
{
	if (newDelay == 0) {
		return;
	}
	else {
		cvarManager->getCvar(DelayCvarName).setValue(newDelay);
		cvarManager->log("ps. DelayCvarName has been set");
	}
}

void PremierSuite::setDisableCasual(bool newDisCasual)
{
	cvarManager->getCvar(disableCasualCvarName).setValue(newDisCasual);
	cvarManager->log("ps. disableCasualCvarName has been set");
}

void PremierSuite::setDisablePrivate(bool newDisPrivate)
{

	cvarManager->getCvar(disablePrivateCvarName).setValue(newDisPrivate);
	cvarManager->log("ps. disablePrivateCvarName has been set");

}

//-----------------------------------------------------------------------------
//--- Instant Queue + Delay Options
//-----------------------------------------------------------------------------

void PremierSuite::queue(ServerWrapper server, void* params, std::string eventName)
{

	float totalQueueDelayTime = 0;
	float autoGGDelayTime = cvarManager->getCvar("ranked_autogg_delay").getFloatValue() / 1000;
	bool autoGG = cvarManager->getCvar("ranked_autogg").getBoolValue();
	float queueDelayTime = cvarManager->getCvar(qDelayCvarName).getFloatValue();
	bool disableCasualQueue = cvarManager->getCvar(disableCasualQCvarName).getBoolValue();
	bool disablePrivate = cvarManager->getCvar(disablePrivateCvarName).getBoolValue();

	if (autoGG) {
		totalQueueDelayTime = queueDelayTime + autoGGDelayTime;
		cvarManager->log("ps. totalQueueDelayTime has been set");
	}
	else {
		totalQueueDelayTime = queueDelayTime;
	}

	if (!server.IsNull() && (disablePrivate || disableCasualQueue))
	{
		auto playlist = (Mode)server.GetPlaylist().GetPlaylistId();

		if ((playlist == CasualChaos || playlist == CasualDoubles || playlist == CasualDuel || playlist == CasualStandard) && disableCasualQueue) {
			return;
		}
		else if ((playlist == Private || playlist == Tournament) && disablePrivate) {
			return;
		}
		else {
			gameWrapper->SetTimeout(std::bind(&PremierSuite::delayedQueue, this), queueDelayTime);
			cvarManager->log("ps. settimeout(delayedQueue, queueDelaytime) has been called");

		}
	}

	gameWrapper->SetTimeout(std::bind(&PremierSuite::delayedQueue, this), queueDelayTime);
	cvarManager->log("ps. settimeout(delayedQueue, queueDelaytime) has been called");
}

void PremierSuite::setDelayedQueue(float newQueueDelayTime)
{
	if (newQueueDelayTime == 0) {
		return;
	}
	else {
		cvarManager->getCvar(qDelayCvarName).setValue(newQueueDelayTime);
	}
}

void PremierSuite::delayedQueue()
{
	auto game = gameWrapper->GetOnlineGame();
	if (game.IsNull()) { cvarManager->log("null_pntr"); return; } //nullcheck

	if (!game.IsNull())
	{
		if (!game.GetbMatchEnded())
		{
			return;
		}
	}

	cvarManager->executeCommand("queue");
	cvarManager->log("PremierSuite queue command executed");
}


//-----------------------------------------------------------------------------
//--- Exit to Freeplay
//-----------------------------------------------------------------------------

void PremierSuite::launchTraining(ServerWrapper server, void* params, std::string eventName)
{
	float totalTrainingDelayTime = 0;
	float trainingDelayTime = cvarManager->getCvar(DelayCvarName).getFloatValue();
	float autoGGDelayTime = cvarManager->getCvar("ranked_autogg_delay").getFloatValue() / 1000;
	bool autoGG = cvarManager->getCvar("ranked_autogg").getBoolValue();
	if (server.IsNull()) { cvarManager->log("null_pntr"); return; } //nullcheck

	if (autoGG) {
		totalTrainingDelayTime = trainingDelayTime + autoGGDelayTime;
		cvarManager->log("ps. training delay time has been set");
	}
	else {
		totalTrainingDelayTime = trainingDelayTime;
		cvarManager->log("ps. training delay time has been set");
	}

	bool disableCasualTraining = cvarManager->getCvar(disableCasualCvarName).getBoolValue();
	bool disablePrivate = cvarManager->getCvar(disablePrivateCvarName).getBoolValue();

	if (!server.IsNull() && (server.GetPlaylist().memory_address != NULL) && (disablePrivate || disableCasualTraining))
		// ! reverses the result, && returns if both statements true, || returns if one statement is true
	{
		auto playlist = (Mode)server.GetPlaylist().GetPlaylistId();

		if ((playlist == CasualChaos || playlist == CasualDoubles || playlist == CasualDuel || playlist == CasualStandard) && disableCasualTraining) {
			return;
		}
		else {
			gameWrapper->SetTimeout(std::bind(&PremierSuite::delayedTraining, this), totalTrainingDelayTime);
			cvarManager->log("ps. settimeout(delayedTraining, totalTrainingDelayTime executed");
		}
	}

	gameWrapper->SetTimeout(std::bind(&PremierSuite::delayedTraining, this), totalTrainingDelayTime);
	cvarManager->log("ps. settimeout(delayedTraining, totalTrainingDelayTime executed");
}

void PremierSuite::delayedTraining()
{
	std::stringstream launchTrainingCommandBuilder;
	std::string mapname = cvarManager->getCvar(trainingMapCvarName).getStringValue();

	if (mapname.compare("random") == 0)
	{
		mapname = gameWrapper->GetRandomMap();
		cvarManager->log("ps. Random map aquired. ");
	}

	launchTrainingCommandBuilder << "start " << mapname << "?Game=TAGame.GameInfo_Tutorial_TA?GameTags=Training";
	const std::string launchTrainingCommand = launchTrainingCommandBuilder.str();
	auto game = gameWrapper->GetOnlineGame();
	if (game.IsNull()) { cvarManager->log("null_pntr"); return; } //nullcheck

	if (!game.IsNull())
	{
		if (!game.GetbMatchEnded())
		{
			return;
		}
	}

	cvarManager->executeCommand("load_freeplay");
	//gameWrapper->ExecuteUnrealCommand(launchTrainingCommand);
	cvarManager->log("PremierSuite launchTrainingCommand executed.");

}

//-----------------------------------------------------------------------------
//--- Exit to Custom Training
//-----------------------------------------------------------------------------

void PremierSuite::launchCustomTraining(ServerWrapper server, void* params, std::string eventName)
{
	float totalCustomTrainingDelayTime = 0;
	float CustomTrainingDelayTime = cvarManager->getCvar(DelayCvarName).getFloatValue();
	float autoGGDelayTime = cvarManager->getCvar("ranked_autogg_delay").getFloatValue() / 1000;
	bool autoGG = cvarManager->getCvar("ranked_autogg").getBoolValue();
	if (server.IsNull()) { cvarManager->log("null_pntr"); return; } //nullcheck

	if (autoGG) {
		totalCustomTrainingDelayTime = CustomTrainingDelayTime + autoGGDelayTime;
	}
	else {
		totalCustomTrainingDelayTime = CustomTrainingDelayTime;
	}

	bool disableCasualTraining = cvarManager->getCvar(disableCasualCvarName).getBoolValue();
	bool disablePrivate = cvarManager->getCvar(disablePrivateCvarName).getBoolValue();

	if (!server.IsNull() && (server.GetPlaylist().memory_address != NULL) && (disablePrivate || disableCasualTraining))
	{
		auto playlist = (Mode)server.GetPlaylist().GetPlaylistId();

		if ((playlist == CasualChaos || playlist == CasualDoubles || playlist == CasualDuel || playlist == CasualStandard) && disableCasualTraining) {
			return;
		}
		else {
			gameWrapper->SetTimeout(std::bind(&PremierSuite::delayedCustomTraining, this), totalCustomTrainingDelayTime);
		}
	}

	gameWrapper->SetTimeout(std::bind(&PremierSuite::delayedCustomTraining, this), totalCustomTrainingDelayTime);
}

void PremierSuite::delayedCustomTraining()
{
	auto game = gameWrapper->GetOnlineGame();
	auto training_code = cvarManager->getCvar("custom_code").getStringValue();

	if (game.IsNull()) { cvarManager->log("null_pntr"); return; }

	if (!game.IsNull())
	{
		if (!game.GetbMatchEnded())
		{
			return;
		}
	}

	cvarManager->executeCommand("load_training " + training_code);
}

//-----------------------------------------------------------------------------
//--- Exit to Workshop Map
//-----------------------------------------------------------------------------

void PremierSuite::launchWorkshop(ServerWrapper server, void* params, std::string eventName)
{
	float totalWorkshopDelayTime = 0;
	float WorkshopDelayTime = cvarManager->getCvar(DelayCvarName).getFloatValue();
	float autoGGDelayTime = cvarManager->getCvar("ranked_autogg_delay").getFloatValue() / 1000;
	bool autoGG = cvarManager->getCvar("ranked_autogg").getBoolValue();
	if (server.IsNull()) { cvarManager->log("null_pntr"); return; } //nullcheck

	if (autoGG) {
		totalWorkshopDelayTime = WorkshopDelayTime + autoGGDelayTime;
	}
	else {
		totalWorkshopDelayTime = WorkshopDelayTime;
	}

	bool disableCasualTraining = cvarManager->getCvar(disableCasualCvarName).getBoolValue();
	bool disablePrivate = cvarManager->getCvar(disablePrivateCvarName).getBoolValue();

	if (!server.IsNull() && (server.GetPlaylist().memory_address != NULL) && (disablePrivate || disableCasualTraining))
	{
		auto playlist = (Mode)server.GetPlaylist().GetPlaylistId();

		if ((playlist == CasualChaos || playlist == CasualDoubles || playlist == CasualDuel || playlist == CasualStandard) && disableCasualTraining) {
			return;
		}
		else {
			gameWrapper->SetTimeout(std::bind(&PremierSuite::delayedWorkshop, this), totalWorkshopDelayTime);
		}
	}

	gameWrapper->SetTimeout(std::bind(&PremierSuite::delayedWorkshop, this), totalWorkshopDelayTime);
}



void PremierSuite::delayedWorkshop()
{
	auto game = gameWrapper->GetOnlineGame();
	std::string workshop_map = cvarManager->getCvar(workshopCvarName).getStringValue();

	if (game.IsNull()) { cvarManager->log("null_pntr"); return; } //nullcheck

	if (!game.IsNull())
	{
		if (!game.GetbMatchEnded())
		{
			return;
		}
	}

	cvarManager->executeCommand("load_workshop " + workshop_map);
	//cvarManager->executeCommand("load_workshop " + currentMap);
}

//-----------------------------------------------------------------------------
//--- Exit to Main Menu
//-----------------------------------------------------------------------------

void PremierSuite::exitGame(ServerWrapper server, void* params, std::string eventName)
{
	float totalExitDelayTime = 0;
	float exitDelayTime = cvarManager->getCvar(DelayCvarName).getFloatValue();
	float autoGGDelayTime = cvarManager->getCvar("ranked_autogg_delay").getFloatValue() / 1000;
	bool autoGG = cvarManager->getCvar("ranked_autogg").getBoolValue();
	if (server.IsNull()) { cvarManager->log("null_pntr"); return; } //nullcheck

	if (autoGG) {
		totalExitDelayTime = exitDelayTime + autoGGDelayTime;
	}
	else {
		totalExitDelayTime = exitDelayTime;
	}

	bool disableCasualExit = cvarManager->getCvar(disableCasualCvarName).getBoolValue();
	bool disablePrivate = cvarManager->getCvar(disablePrivateCvarName).getBoolValue();

	if (!server.IsNull() && (disablePrivate || disableCasualExit))
	{
		auto playlist = (Mode)server.GetPlaylist().GetPlaylistId();

		if ((playlist == CasualChaos || playlist == CasualDoubles || playlist == CasualDuel || playlist == CasualStandard) && disableCasualExit) {
			return;
		}
		else if ((playlist == Private || playlist == Tournament) && disablePrivate) {
			return;
		}
		else {
			gameWrapper->SetTimeout(std::bind(&PremierSuite::delayedExit, this), totalExitDelayTime);
		}
	}

	gameWrapper->SetTimeout(std::bind(&PremierSuite::delayedExit, this), totalExitDelayTime);
}

void PremierSuite::setExitEnabled(bool newExitEnabled)
{
	cvarManager->getCvar(exitCvarName).setValue(newExitEnabled);
}

void PremierSuite::delayedExit()
{
	auto game = gameWrapper->GetOnlineGame();
	if (game.IsNull()) { cvarManager->log("null_pntr"); return; } //nullcheck

	if (!game.IsNull())
	{
		if (!game.GetbMatchEnded())
		{
			return;
		}
	}

	cvarManager->executeCommand("unreal_command disconnect");
}

//-----------------------------------------------------------------------------
//--- Keybind Helpers
//-----------------------------------------------------------------------------

/// <summary>
/// Quick-change if plugin is enabled, sets cvar to opposite it's current value 
/// </summary>
void PremierSuite::quickPluginEnabled()
{
	bool pluginEnabled = cvarManager->getCvar("plugin_enabled").getBoolValue();
	if (pluginEnabled == false) {
		cvarManager->getCvar("plugin_enabled").setValue(true);
	}

	if (pluginEnabled == true) {
		cvarManager->getCvar("plugin_enabled").setValue(false);
	}
}

/// <summary>Checks if the GUI window is bound.</summary>
/// <param name="windowName">Name of the GUI window</param>
/// <returns>Bool with if the GUI window is bound</returns>
bool IsGUIWindowBound(const std::string& windowName)
{
	const std::string bind = "togglemenu " + windowName;
	std::ifstream file(BINDS_FILE_PATH);
	if (file.is_open()) {
		std::string line;
		while (getline(file, line)) {
			if (line.find(bind) != std::string::npos) {
				file.close();
				return true;
			}
		}
		file.close();
	}

	return false;
}

// <summary>Get current keybinds for GUI.</summary>
// <param name="windowName">Name of the GUI window</param>
// <returns> Vector of std::string keybinds.</returns>
std::vector<std::string> PremierSuite::GetGUIKeyFromBindsConfig(const std::string windowName, bool log = false)
{
	const std::string bind = "togglemenu " + windowName;
	std::ifstream file(BINDS_FILE_PATH);
	std::vector<std::string> lines;

	if (file.is_open()) {
		std::string line;
		std::string word;
		while (getline(file, line)) {
			if (line.find(bind) != std::string::npos) {
				std::vector<std::string> x = split(line, ' ');
				lines.push_back(x[1]);
				if (log) {
					cvarManager->log(x[0]);
					cvarManager->log(x[1]);
					cvarManager->log(x[2]);
				}
			}
		}
		file.close();
		return std::vector<std::string>(lines);
	}
	cvarManager->log("No keybinds for " + windowName + " found.");
	return {};
}

// <summary>Get number of keybinds present in binds.cfg.</summary>
// <returns> Int of number of bound keybinds.</returns>
int PremierSuite::GetBoundConfigKeybinds()
{
	std::vector<std::string> vctKeybinds = GetGUIKeyFromBindsConfig(GetMenuName());
	return static_cast<int>(vctKeybinds.size());
}

// <summary>Check correct keybinds are set from CFG onLoad()</summary>
void PremierSuite::handleKeybindCvar() {
	std::vector<std::string> vectorKeybinds = GetGUIKeyFromBindsConfig(GetMenuName());

	if (vectorKeybinds.empty()) {
		cvarManager->log("No premiersuite in cfg, setting default: F3");
		cvarManager->setBind(DEFAULT_GUI_KEYBIND, "togglemenu " + GetMenuName());
	}
	if ((size_t)vectorKeybinds.size() == 1) {
		cvarManager->log("Setting keybind" + vectorKeybinds[0]);
		cvarManager->setBind(vectorKeybinds[0], "togglemenu " + GetMenuName());
		
	}
	else {
		bool bound = false;
		// Loop through each keybind, remove default and add currently bound
		for (std::string bind : vectorKeybinds) {
			if (bound == false) {
				if (bind == "F3") { cvarManager->removeBind(bind); }
				else {
					cvarManager->setBind(bind, "togglemenu premiersuite");
					bound = true;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
//--- Server Hooks
//-----------------------------------------------------------------------------

void PremierSuite::onMatchEnd(ServerWrapper server, void* params, std::string eventName)
{
	
	const bool queueEnabled = cvarManager->getCvar(queueCvarName).getBoolValue();
	const bool freeplayEnabled = cvarManager->getCvar("freeplay_enabled").getBoolValue();
	const bool customtrainingEnabled = cvarManager->getCvar("custom_enabled").getBoolValue();
	const bool workshopEnabled = cvarManager->getCvar("workshop_enabled").getBoolValue();
	const bool exitEnabled = cvarManager->getCvar("exit_enabled").getBoolValue();
	if (server.IsNull()) { cvarManager->log("null_pntr"); return; }

	if (queueEnabled) {
		queue(server, params, eventName);
		if (server.IsNull()) { cvarManager->log("null_pntr"); return; } 
	}
	if (exitEnabled) {
		exitGame(server, params, eventName);
		if (server.IsNull()) { cvarManager->log("null_pntr"); return; }
	}
	else {
		if (freeplayEnabled) {
			launchTraining(server, params, eventName);
			if (server.IsNull()) { cvarManager->log("null_pntr"); return; }
		}
		else
			if (customtrainingEnabled) {
				launchCustomTraining(server, params, eventName);
				if (server.IsNull()) { cvarManager->log("null_pntr"); return; } 
			}
			else
				if (workshopEnabled) {
					launchWorkshop(server, params, eventName);
					if (server.IsNull()) { cvarManager->log("null_pntr"); return; }
				}
	}
}

void PremierSuite::hookMatchEnded()
{
	gameWrapper->HookEventWithCaller<ServerWrapper>(matchEndedEvent, std::bind(&PremierSuite::onMatchEnd, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	hooked = true;
	logHookType("Hooked");
}

void PremierSuite::unhookMatchEnded()
{
	gameWrapper->UnhookEvent(matchEndedEvent);
	hooked = false;
	logHookType("Unhooked");
}

void PremierSuite::logHookType(const char* const hookType) const
{
	std::stringstream logBuffer;
	logBuffer << hookType << " match ended event.";
	cvarManager->log(logBuffer.str());
}

void PremierSuite::writePlugins() {
	
	cvarManager->executeCommand("plugin unload instantsuite");
}

void PremierSuite::onLoad()
{
	
	_globalCvarManager = cvarManager;

	//-----------------------------------------------------------------------------
	// File Helper Cvars
	//-----------------------------------------------------------------------------

	BakkesModConfigFolder = gameWrapper->GetBakkesModPath() / L"cfg";
	std::filesystem::path BakkesModCrashesFolder = gameWrapper->GetBakkesModPath() / L"crashes";
	if (!exists(BakkesModCrashesFolder)) {
		std::filesystem::create_directory(BakkesModCrashesFolder);
		LOG("Data folder created: {}", BakkesModCrashesFolder.u8string());

	}
	std::filesystem::path PremierSuiteDataFolder = gameWrapper->GetDataFolder() / L"premiersuite";
	if (!exists(PremierSuiteDataFolder)) {
		std::filesystem::create_directory(PremierSuiteDataFolder);
		LOG("Data folder created: {}", PremierSuiteDataFolder.u8string());
	}

	RocketLeagueExecutableFolder = std::filesystem::current_path();

	//-----------------------------------------------------------------------------
	// CVar registration
	//-----------------------------------------------------------------------------

	enabled = std::make_shared<bool>(true);
	freeplayEnabled = std::make_shared<bool>(false);
	customEnabled = std::make_shared<bool>(false);
	workshopEnabled = std::make_shared<bool>(false);
	exitEnabled = std::make_shared<bool>(false);

	keybind = std::make_shared<std::string>("F3");
	customCode = std::make_shared<std::string>("F3");

	cvarManager->registerCvar("plugin_enabled", "1", "Enable PremierSuite", true, true, 0, true, 1).bindTo(enabled);
	cvarManager->getCvar("plugin_enabled").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			if (cvar.getStringValue() == "1") {
				LOG("PremierSuite: Enabled");
			};
			if (cvar.getStringValue() == "0") {
				LOG("PremierSuite: Disabled");
			};
		}
	);

	cvarManager->registerCvar("freeplay_enabled", "0", "Enable Instant Exit -> Freeplay", true, true, 0, true, 1).bindTo(freeplayEnabled);
	cvarManager->getCvar("freeplay_enabled").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			if (cvar.getStringValue() == "1") {
				LOG("Exit -> Freeplay: Enabled");
			};
			if (cvar.getStringValue() == "0") {
				LOG("Exit -> Freeplay: Disabled");
			};
		}
	);

	cvarManager->registerCvar("custom_enabled", "0", "Enable Instant Exit -> Custom Training", true, true, 0, true, 1).bindTo(customEnabled);
	cvarManager->getCvar("custom_enabled").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			if (cvar.getStringValue() == "1") {
				LOG("Exit -> Custom Training: Enabled");
			};
			if (cvar.getStringValue() == "0") {
				LOG("Exit -> Custom Training: Disabled");
			};
		}
	);

	cvarManager->registerCvar("exit_enabled", "0", "Enable Instant Exit -> Main Menu", true, true, 0, true, 1).bindTo(exitEnabled);
	cvarManager->getCvar("exit_enabled").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			if (cvar.getStringValue() == "1") {
				LOG("Exit -> Main Menu: Enabled");
			};
			if (cvar.getStringValue() == "0") {
				LOG("Exit -> Main Menu: Disabled");
			};
		}
	);

	cvarManager->registerCvar("workshop_enabled", "0", "Enable Instant Exit -> Main Menu", true, true, 0, true, 1).bindTo(workshopEnabled);
	cvarManager->getCvar("workshop_enabled").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			if (cvar.getStringValue() == "1") {
				LOG("Exit -> Workshop: Enabled");
			};
			if (cvar.getStringValue() == "0") {
				LOG("Exit -> Workshop: Disabled");
			};
		}
	);
	
	cvarManager->registerCvar("plugin_keybind", "F3", "GUI Keybind", true, true, 0, true, 1).bindTo(keybind);
	cvarManager->getCvar("plugin_keybind").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			LOG("Keybind {} -> {}", oldValue, cvar.getStringValue());
		}
	);

	cvarManager->registerCvar("custom_code", "A0FE-F860-967D-E628", "Custom-training code.", true, true, 0, true, 1).bindTo(customCode);
	cvarManager->getCvar("plugin_keybind").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		LOG("Keybind {} -> {}", oldValue, cvar.getStringValue());
		}
	);

	cvarManager->registerCvar(disablePrivateCvarName, "0", "Disable plugin during Private, Tournament, and Heatseeker matches.");
	cvarManager->registerCvar(DelayCvarName, "0", "Seconds to wait before loading into training mode.");

	//Training Cvars
	cvarManager->registerCvar(trainingMapCvarName, "Beckwith Park (Stormy)", "Determines the map that will launch for training.");
	cvarManager->registerCvar(disableCasualCvarName, "0", "Don't automatically go to training when ending a casual game.");

	cvarManager->registerCvar(workshopCvarName, "", "Desired workshop map.");

	workshopMapDirPath = std::make_shared<std::string>();
	cvarManager->registerCvar("ps_workshop_path", WORKSHOP_MAPS_PATH.string(),
		"Default path for your workshop maps directory").bindTo(workshopMapDirPath);

	customMapDirPath = std::make_shared<std::string>();
	cvarManager->registerCvar("ps_custom_path", CUSTOM_MAPS_PATH.string(),
		"Default path for your custom maps directory").bindTo(customMapDirPath);

	//Exit Cvars
	cvarManager->registerCvar(exitCvarName, "0", "Instantly exit to main menu instead of training at end of match.");

	//Queue cvars
	cvarManager->registerCvar(queueCvarName, "0", "Instantly queue for previously selected playlists at end of match.");
	cvarManager->registerCvar(qDelayCvarName, "0", "Seconds to wait before starting queue.");
	cvarManager->registerCvar(disableCasualQCvarName, "0", "Don't automatically queue when ending a casual game.");

	//Keybind Cvars
	cvarManager->registerNotifier("keybind_notification", [this](std::vector<std::string> args) {
		std::string keybind;
	if (args.size() == 1) {
		keybind = args[0];
		gameWrapper->Toast("Keybind: " + keybind, "", "default", 3.5F, 0U, 300.0F);
	}
	else {
		cvarManager->log("Too many args!");
	}
		}, "", PERMISSION_ALL);

	cvarManager->registerNotifier("debug", [this](std::vector<std::string> args) {
		std::vector<std::string> vectorKeybinds = GetGUIKeyFromBindsConfig(GetMenuName(), false);

	if (vectorKeybinds.empty()) {
		cvarManager->log("setBind(DEFAULT_GUI_KEYBIND");
	}
	if ((size_t)vectorKeybinds.size() == 1) {
		cvarManager->log("Set to t[1]: " + vectorKeybinds[1]);
	}
	else {
		int i = -1;
		bool bound = false;
		// Loop through each keybind, remove default and add currently bound

		for (std::string bind : vectorKeybinds) {
			if (bound == false) {
				if (bind == "F3") { cvarManager->log("Else - set to F3: " + bind); }
				else {
					cvarManager->log("Else - set to bind: " + bind);
					bound = true;
				}
			}
		}
	}
		}, "", PERMISSION_ALL);

	// Set the window bind to the default keybind if is not set.
	handleKeybindCvar();

	gameWrapper->SetTimeout([this](GameWrapper* gw) 
		{
		this->writePlugins();
		}, 10);

	hookMatchEnded();
}

void PremierSuite::onUnload()
{
	{
	}
}
