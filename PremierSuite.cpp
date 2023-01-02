#include "pch.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <Windows.h>
#include <filesystem>
#include "PremierSuite.h"
#include "GuiBase.h"
//#include "enums.h"

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

std::string btos(bool x)
{
	if (x) return "True";
	return "False";
}

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
//--- Server Hook Setters
//-----------------------------------------------------------------------------

bool PremierSuite::isRanked(ServerWrapper server) {
	if (server.GetPlaylist().GetbRanked() == 1) { return true; };
	return false;
}

bool PremierSuite::isPrivate(ServerWrapper server) {
	return server.GetPlaylist().IsPrivateMatch();
}

bool PremierSuite::isTournament(ServerWrapper server) {
	return server.GetPlaylist().IsPrivateMatch();
}

bool PremierSuite::isStandard(ServerWrapper server) {
	if (server.GetPlaylist().GetbStandard() == 1) { return true; };
	return false;
}

void addHandler(std::function<void(int)> callback)
{

}


void PremierSuite::queue(ServerWrapper server, void* params, std::string eventName)
{
	float delay = 0;
	float delay = 0;
	if (server.IsNull()) { return; }
	if (*autoGG) { delay = *delayExit + *autoGGDelay; }
	else { delay = *delayExit; }
	if (isTournament(server)) {
		return;
	}
	if (!server.IsNull() && (server.GetPlaylist().memory_address != NULL) && (*disablePrivate || *disableExitCasual))
	{
		if (isStandard(server) && *disableExitCasual) {
			return;
		}
		if (isPrivate(server) && *disablePrivate) {
			return;
		}
		else {
			gameWrapper->SetTimeout(std::bind(&PremierSuite::delayedQueue, this), delay);
		}
	}
	gameWrapper->SetTimeout(std::bind(&PremierSuite::delayedQueue, this), delay);
}

void PremierSuite::delayedQueue()
{
	auto game = gameWrapper->GetOnlineGame();
	if (game.IsNull()) { return; }
	if (!game.IsNull())
	{
		if (!game.GetbMatchEnded())
		{
			return;
		}
	}
	cvarManager->executeCommand("queue");
}

void PremierSuite::launchFreeplay(ServerWrapper server, void* params, std::string eventName)
{
	float delay = 0;
	if (server.IsNull()) { return; } 
	if (*autoGG) { delay = *delayExit + *autoGGDelay; }
	else { delay = *delayExit; }

	if (isTournament(server)) {
		return;
	}

	if (!server.IsNull() && (server.GetPlaylist().memory_address != NULL) && (*disablePrivate || *disableExitCasual))
	{
		if (isStandard(server) && *disableExitCasual) {
			return;
		}
		if (isPrivate(server) && *disablePrivate) {
			return;
		}
		else {
			gameWrapper->SetTimeout(std::bind(&PremierSuite::delayedFreeplay, this), delay);
		}
	}
	gameWrapper->SetTimeout(std::bind(&PremierSuite::delayedFreeplay, this), delay);
}

void PremierSuite::callbackSetDelay(ServerWrapper server, void* params, std::string eventName, std::function<void(PremierSuite)> &callback) {
	float delay = 0;
	if (server.IsNull()) { return; }
	if (*autoGG) { delay = *delayExit + *autoGGDelay; }
	else { delay = *delayExit; }

	if (isTournament(server)) {
		return;
	}
	delayedFreeplay();
	if (!server.IsNull() && (server.GetPlaylist().memory_address != NULL) && (*disablePrivate || *disableExitCasual))
	{
		if (isStandard(server) && *disableExitCasual) {
			return;
		}
		if (isPrivate(server) && *disablePrivate) {
			return;
		}
		else {
			gameWrapper->SetTimeout(std::bind(&delayedFreeplay, this), delay);
		}
	}
	gameWrapper->SetTimeout(std::bind(&callback, this), delay);

}


void PremierSuite::delayedFreeplay()
{
	std::stringstream launchTrainingCommandBuilder;
	std::string mapname = cvarManager->getCvar("freeplay_map").getStringValue();

	// TODO: Allow random mapname
	if (mapname.compare("random") == 0)
	{
		mapname = gameWrapper->GetRandomMap();
	}

	launchTrainingCommandBuilder << "start " << mapname << "?Game=TAGame.GameInfo_Tutorial_TA?GameTags=Training";
	const std::string launchTrainingCommand = launchTrainingCommandBuilder.str();
	auto game = gameWrapper->GetOnlineGame();
	if (!game.IsNull())
	{
		if (!game.GetbMatchEnded())
		{
			return;
		}
	}
	gameWrapper->ExecuteUnrealCommand(launchTrainingCommand);
}

void PremierSuite::launchCustomTraining(ServerWrapper server, void* params, std::string eventName)
{
	float totalCustomTrainingDelayTime = 0;
	float CustomTrainingDelayTime = cvarManager->getCvar("exit_delay").getFloatValue();
	float autoGGDelayTime = cvarManager->getCvar("ranked_autogg_delay").getFloatValue() / 1000;
	bool autoGG = cvarManager->getCvar("ranked_autogg").getBoolValue();
	if (server.IsNull()) { cvarManager->log("null_pntr"); return; } //nullcheck

	if (autoGG) {
		totalCustomTrainingDelayTime = CustomTrainingDelayTime + autoGGDelayTime;
	}
	else {
		totalCustomTrainingDelayTime = CustomTrainingDelayTime;
	}

	bool disableCasualTraining = cvarManager->getCvar("disable_exit_casual").getBoolValue();
	bool disablePrivate = cvarManager->getCvar("disable_private").getBoolValue();

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

void PremierSuite::launchWorkshop(ServerWrapper server, void* params, std::string eventName)
{
	float totalWorkshopDelayTime = 0;
	float WorkshopDelayTime = cvarManager->getCvar("disable_exit_delay").getFloatValue();
	float autoGGDelayTime = cvarManager->getCvar("ranked_autogg_delay").getFloatValue() / 1000;
	bool autoGG = cvarManager->getCvar("ranked_autogg").getBoolValue();
	if (server.IsNull()) { cvarManager->log("null_pntr"); return; } //nullcheck

	if (autoGG) {
		totalWorkshopDelayTime = WorkshopDelayTime + autoGGDelayTime;
	}
	else {
		totalWorkshopDelayTime = WorkshopDelayTime;
	}

	bool disableCasualTraining = cvarManager->getCvar("disable_exit_casual").getBoolValue();
	bool disablePrivate = cvarManager->getCvar("disable_private").getBoolValue();

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
	std::string workshop_map = cvarManager->getCvar("workshop_map").getStringValue();

	if (game.IsNull()) { cvarManager->log("null_pntr"); return; } //nullcheck

	if (!game.IsNull())
	{
		if (!game.GetbMatchEnded())
		{
			return;
		}
	}

	cvarManager->executeCommand("load_workshop " + workshop_map);
}

void PremierSuite::exitGame(ServerWrapper server, void* params, std::string eventName)
{
	float totalExitDelayTime = 0;
	float exitDelayTime = cvarManager->getCvar("exit_delay").getFloatValue();
	float autoGGDelayTime = cvarManager->getCvar("ranked_autogg_delay").getFloatValue() / 1000;
	bool autoGG = cvarManager->getCvar("ranked_autogg").getBoolValue();
	if (!server) { return; }

	if (autoGG) {
		totalExitDelayTime = exitDelayTime + autoGGDelayTime;
	}
	else {
		totalExitDelayTime = exitDelayTime;
	}

	bool disableCasualExit = cvarManager->getCvar("disable_exit_casual").getBoolValue();
	bool disablePrivate = cvarManager->getCvar("disable_private").getBoolValue();

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
//--- CVar Functions
//-----------------------------------------------------------------------------

void PremierSuite::quickPluginEnabled()
{
	if (*enabled == false) { cvarManager->getCvar("plugin_enabled").setValue(true); }
	else { cvarManager->getCvar("plugin_enabled").setValue(false); }
}

void PremierSuite::setEnablePlugin(std::shared_ptr<bool> newBool)
{
	_globalCvarManager->getCvar("plugin_enabled").setValue(*newBool);
}

void PremierSuite::setEnableFreeplay(std::shared_ptr<bool> newBool)
{
	_globalCvarManager->getCvar("freeplay_enabled").setValue(*newBool);
	_globalCvarManager->getCvar("custom_enabled").setValue(false);
	_globalCvarManager->getCvar("workshop_enabled").setValue(false);
	_globalCvarManager->getCvar("exit_enabled").setValue(false);
}

void PremierSuite::setEnableCustomTraining(std::shared_ptr<bool> newBool)
{
	_globalCvarManager->getCvar("custom_enabled").setValue(*newBool);
	_globalCvarManager->getCvar("freeplay_enabled").setValue(false);
	_globalCvarManager->getCvar("workshop_enabled").setValue(false);
	_globalCvarManager->getCvar("exit_enabled").setValue(false);
}

void PremierSuite::setEnableWorkshop(std::shared_ptr<bool> newBool)
{
	_globalCvarManager->getCvar("workshop_enabled").setValue(*newBool);
	_globalCvarManager->getCvar("freeplay_enabled").setValue(false);
	_globalCvarManager->getCvar("custom_enabled").setValue(false);
	_globalCvarManager->getCvar("exit_enabled").setValue(false);
}

void PremierSuite::setEnableExit(std::shared_ptr<bool> newBool)
{
	_globalCvarManager->getCvar("exit_enabled").setValue(*newBool);
	_globalCvarManager->getCvar("freeplay_enabled").setValue(false);
	_globalCvarManager->getCvar("custom_enabled").setValue(false);
	_globalCvarManager->getCvar("workshop_enabled").setValue(false);
}

void PremierSuite::setEnableQueue(std::shared_ptr<bool> newBool)
{
	_globalCvarManager->getCvar("queue_enabled").setValue(*newBool);
}

void PremierSuite::setDisableQueueCasual(std::shared_ptr<bool> newBool)
{
	_globalCvarManager->getCvar("disable_queue_casual").setValue(*newBool);
}

void PremierSuite::setDisableExitCasual(std::shared_ptr<bool> newBool)
{
	_globalCvarManager->getCvar("disable_exit_casual").setValue(*newBool);
}

void PremierSuite::setDisablePrivate(std::shared_ptr<bool> newBool)
{
	_globalCvarManager->getCvar("disable_private").setValue(*newBool);
}

void PremierSuite::setDelayQueue(std::shared_ptr<float>  newFloat)
{
	_globalCvarManager->getCvar("queue_delay").setValue(*newFloat);
}

void PremierSuite::setDelayExit(std::shared_ptr<float> newFloat)
{
	_globalCvarManager->getCvar("exit_delay").setValue(*newFloat);
}

void PremierSuite::setCustomTrainingCode(char newCode)
{
	_globalCvarManager->getCvar("custom_code").setValue(newCode);
}

void PremierSuite::setCustomTrainingCode(std::string newCode)
{
	_globalCvarManager->getCvar("custom_code").setValue(newCode);
}

void PremierSuite::setNewGUIKeybind(std::string newKeybind)
{
	_globalCvarManager->getCvar("ps_gui_keybind").setValue(newKeybind);
}

void PremierSuite::setNewPluginKeybind(std::string newKeybind)
{
	_globalCvarManager->getCvar("ps_enabled_keybind").setValue(newKeybind);
}

//-----------------------------------------------------------------------------
//--- Keybind helpers
//-----------------------------------------------------------------------------

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

// <summary>Return lines containing the given search string.</summary>
// <param name="searchString">Name of the string to search</param>
// <returns> Vector of std::string keybinds.</returns>
std::vector<std::string> PremierSuite::parseCfg(const std::string searchString, bool log)
{
	const std::string bind = searchString;
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
	return {};
}

// <summary>Check correct keybinds are set from CFG onLoad()</summary>
void PremierSuite::handleKeybindCvar() {
#ifdef DEBUG
	std::vector<std::string> guiKeybinds = parseCfg("togglemenu PremierSuite", false);
	std::vector<std::string> pluginKeybinds = parseCfg("change_ps_enabled", false);
#else 
	std::vector<std::string> guiKeybinds = parseCfg("togglemenu PremierSuite", true);
	std::vector<std::string> pluginKeybinds = parseCfg("change_ps_enabled", true);
#endif
	if (guiKeybinds.empty()) {
		cvarManager->log("Setting default GUI keybind: F3");
		cvarManager->setBind("F3", "togglemenu " + GetMenuName());
		*gui_keybind = "F3";
	}
	if ((size_t)guiKeybinds.size() == 1) {
		cvarManager->log("Setting gui keybind from binds.cfg file " + guiKeybinds[0]);
		cvarManager->setBind(guiKeybinds[0], "togglemenu " + GetMenuName());
		cvarManager->getCvar("ps_gui_keybind").setValue(guiKeybinds[0]);
		*gui_keybind = guiKeybinds[0];
	}
	else {
		bool guibound = false;
		for (std::string bind : guiKeybinds) {
			if (guibound == false) {
				if (bind == "F3") { cvarManager->removeBind(bind); } // if multiple entries, likely dont want the default F3
				else {
					cvarManager->setBind(bind, "togglemenu premiersuite");
					cvarManager->getCvar("ps_gui_keybind").setValue(bind);
					guibound = true;
				}
			}
		}
	}

	if (pluginKeybinds.empty()) { cvarManager->log("Toggle Keybind is Unset."); }
	if ((size_t)pluginKeybinds.size() == 1) {
		cvarManager->log("Setting toggle plugin keybind from binds.cfg file " + pluginKeybinds[0]);
		cvarManager->setBind(pluginKeybinds[0], "change_ps_enabled");
		cvarManager->getCvar("ps_enabled_keybind").setValue(pluginKeybinds[0]);
	}
	else {
		bool pluginbound = false;
		for (std::string bind : pluginKeybinds) {
			if (pluginbound == false) {
				if (bind == "Unset") { cvarManager->removeBind(bind); } //remove unset binding, shouldnt happen
				else {
					cvarManager->setBind(bind, "change_ps_enabled");
					cvarManager->getCvar("ps_enabled_keybind").setValue(bind);
					pluginbound = true;
				}
			}
		}
	}
}

void PremierSuite::checkConflicts()
{
	// instantsuite functions cause weird bad stuff no good 
	std::vector<std::string> conflicts = parseCfg("instantsuite", false);
	if (!conflicts.empty()) {
		cvarManager->executeCommand("unload instantsuite");
	}
}

//-----------------------------------------------------------------------------
//--- Server Hooks
//-----------------------------------------------------------------------------

void PremierSuite::onMatchEnd(ServerWrapper server, void* params, std::string eventName)
{
	if (server.IsNull()) { return; }

	cvarManager->log("------------------------------------------------");
	LOG("GetID {}", btos(isRanked(server)));
	cvarManager->log("------------------------------------------------");

	if (*enableQueue) {
		queue(server, params, eventName);
		if (server.IsNull()) {  return; }
	}
	if (*exitEnabled) {
		exitGame(server, params, eventName);
		if (server.IsNull()) {  return; }
	}
	else {
		if (*freeplayEnabled) {
			launchFreeplay(server, params, eventName);
			if (server.IsNull()) {  return; }
		}
		else
			if (*customEnabled) {
				launchCustomTraining(server, params, eventName);
				if (server.IsNull()) {  return; }
			}
			else
				if (*workshopEnabled) {
					launchWorkshop(server, params, eventName);
					if (server.IsNull()) { return; }
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

void PremierSuite::onLoad()
{
	_globalCvarManager = cvarManager;

	//-----------------------------------------------------------------------------
	// File Helper Cvars
	//-----------------------------------------------------------------------------

	BakkesModConfigFolder = gameWrapper->GetBakkesModPath() / L"cfg";

	std::filesystem::path PremierSuiteDataFolder = gameWrapper->GetDataFolder() / L"premiersuite";
	if (!exists(PremierSuiteDataFolder)) {
		std::filesystem::create_directory(PremierSuiteDataFolder);
	}

	RocketLeagueExecutableFolder = std::filesystem::current_path();

	//-----------------------------------------------------------------------------
	// CVar registration
	//-----------------------------------------------------------------------------

	enabled = std::make_shared<bool>(true);
	cvarManager->registerCvar("plugin_enabled", "1", "Enable PremierSuite").bindTo(enabled);
	cvarManager->getCvar("plugin_enabled").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*enabled = cvar.getBoolValue();
		}
	);

	enableQueue = std::make_shared<bool>(false);
	cvarManager->registerCvar("queue_enabled", "1", "Don't automatically queue when ending a casual game.").bindTo(enableQueue);
	cvarManager->getCvar("queue_enabled").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*enableQueue = cvar.getBoolValue();
		}
	);

	autoGG = std::make_shared<bool>(false);
	cvarManager->registerCvar("ranked_autogg", "0", "Automatically say GG after the game.").bindTo(autoGG);
	cvarManager->getCvar("ranked_autogg").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*autoGG = cvar.getBoolValue();
		}
	);

	autoGGDelay = std::make_shared<float>(1);
	cvarManager->registerCvar("ranked_autogg_delay", "0", "Delay for GG after the game.", true, true, 0, true, 5).bindTo(autoGGDelay);
	cvarManager->getCvar("ranked_autogg_delay").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*autoGGDelay = cvar.getFloatValue();
		}
	);

	freeplayEnabled = std::make_shared<bool>(false);
	cvarManager->registerCvar("freeplay_enabled", "0", "Enable Instant Exit -> Freeplay").bindTo(freeplayEnabled);
	cvarManager->getCvar("freeplay_enabled").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			*freeplayEnabled = cvar.getBoolValue();

		}
	);

	customEnabled = std::make_shared<bool>(false);
	cvarManager->registerCvar("custom_enabled", "0", "Enable Instant Exit -> Custom Training").bindTo(customEnabled);
	cvarManager->getCvar("custom_enabled").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*customEnabled = cvar.getBoolValue();

		}
	);

	exitEnabled = std::make_shared<bool>(false);
	cvarManager->registerCvar("exit_enabled", "0", "Enable Instant Exit -> Main Menu").bindTo(exitEnabled);
	cvarManager->getCvar("exit_enabled").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			*exitEnabled = cvar.getBoolValue();
		}
	);

	workshopEnabled = std::make_shared<bool>(false);
	cvarManager->registerCvar("workshop_enabled", "0", "Enable Instant Exit -> Main Menu").bindTo(workshopEnabled);
	cvarManager->getCvar("workshop_enabled").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			*workshopEnabled = cvar.getBoolValue();
		}
	);

	gui_keybind = std::make_shared<std::string>("F3");
	cvarManager->registerCvar("ps_gui_keybind", "F3", "Keybind to open the GUI").bindTo(gui_keybind);
	cvarManager->getCvar("ps_gui_keybind").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			*gui_keybind = cvar.getStringValue();
		}
	);

	plugin_keybind = std::make_shared<std::string>("Unset");
	cvarManager->registerCvar("ps_enabled_keybind", "Unset", "Enable/disable plugin Keybind").bindTo(plugin_keybind);
	cvarManager->getCvar("ps_enabled_keybind").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*plugin_keybind = cvar.getStringValue();
		}
	);

	customCode = std::make_shared<std::string>("A0FE-F860-967D-E628"); //TODO: check which sets default val
	cvarManager->registerCvar("custom_code", "A0FE-F860-967D-E628", "Custom-training code.").bindTo(customCode);
	cvarManager->getCvar("custom_code").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
			*customCode = cvar.getStringValue();
		}
	);

	disablePrivate = std::make_shared<bool>(true);
	cvarManager->registerCvar("disable_private", "1", "Disable plugin during Private, Tournament, and Heatseeker matches.").bindTo(disablePrivate);
	cvarManager->getCvar("disable_private").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*disablePrivate = cvar.getBoolValue();
		}
	);

	delayExit = std::make_shared<float>(0);
	cvarManager->registerCvar("exit_delay", "0", "Seconds to wait before loading into training mode (0 to 10 seconds).", true, true, 0, true, 10).bindTo(delayExit);
	cvarManager->getCvar("exit_delay").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*delayExit = cvar.getFloatValue();
		}
	);

	delayQueue = std::make_shared<float>(0);
	cvarManager->registerCvar("queue_delay", "0", "Seconds to wait before starting queue (0 to 10 seconds).", true, true, 0, true, 10).bindTo(delayQueue);
	cvarManager->getCvar("queue_delay").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*delayQueue = cvar.getFloatValue();
		}
	);

	freeplayMap = std::make_shared<std::string>("Beckwith Park (Stormy)");
	cvarManager->registerCvar("freeplay_map", "Beckwith Park (Stormy)", "Determines the map that will launch for training.").bindTo(freeplayMap);
	cvarManager->getCvar("freeplay_map").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*freeplayMap = cvar.getStringValue();
		}
	);

	disableExitCasual = std::make_shared<bool>(false);
	cvarManager->registerCvar("disable_exit_casual", "1", "Don't automatically exit when ending a casual game.").bindTo(disableExitCasual);
	cvarManager->getCvar("disable_exit_casual").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*disableExitCasual = cvar.getBoolValue();
		}
	);

	disableQueueCasual = std::make_shared<bool>(false);
	cvarManager->registerCvar("disable_queue_casual", "0", "Don't automatically queue when ending a casual game.").bindTo(disableQueueCasual);
	cvarManager->getCvar("disable_queue_casual").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*disableQueueCasual = cvar.getBoolValue();
		}
	);

	workshopMap = std::make_shared<std::string>();
	cvarManager->registerCvar("workshop_map", "", "Map to load into workshop.", true, true, 0, true, 1).bindTo(workshopMap);
	cvarManager->getCvar("workshop_map").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*workshopMap = cvar.getStringValue();
		}
	);

	workshopMapDirPath = std::make_shared<std::string>();
	cvarManager->registerCvar("ps_workshop_path", WORKSHOP_MAPS_PATH.string(),
		"Default path for your workshop maps directory").bindTo(workshopMapDirPath);

	customMapDirPath = std::make_shared<std::string>();
	cvarManager->registerCvar("ps_custom_path", CUSTOM_MAPS_PATH.string(),
		"Default path for your custom maps directory").bindTo(customMapDirPath);

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
		cvarManager->log(*plugin_keybind);


	}, "", PERMISSION_ALL);

	cvarManager->registerNotifier("change_ps_enabled", [this](std::vector<std::string> args) {

			quickPluginEnabled();
	
		}, "", PERMISSION_ALL);

	gameWrapper->SetTimeout([this](GameWrapper* gw)
		{
			this->handleKeybindCvar();

		}, 1);
	hookMatchEnded();
}

