#include "pch.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <Windows.h>
#include <filesystem>
#include "PremierSuite.h"
#include "GuiBase.h"

#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))

BAKKESMOD_PLUGIN(PremierSuite, "Premier Suite", plugin_version, PLUGINTYPE_FREEPLAY | PLUGINTYPE_CUSTOM_TRAINING)

std::filesystem::path BakkesModConfigFolder;
std::filesystem::path PremierSuiteDataFolder;
std::filesystem::path RocketLeagueExecutableFolder;
std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

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

//-----------------------------------------------------------------------------
//--- Game Status Functions
//-----------------------------------------------------------------------------

bool PremierSuite::isRanked(ServerWrapper server) {
	if (server.GetPlaylist().GetbRanked() == 1) { return true; };
	return false;
}

bool PremierSuite::isPrivate(ServerWrapper server) {
	return server.GetPlaylist().IsPrivateMatch();
}

bool PremierSuite::isTournament(ServerWrapper server) {
	return server.GetPlaylist().IsTournamentMatch();
}

bool PremierSuite::isStandard(ServerWrapper server) {
	if (server.GetPlaylist().GetbStandard() == 1) { return true; };
	return false;
}

std::string PremierSuite::getInGameMap()
{
	return gameWrapper->GetCurrentMap();
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

void PremierSuite::setFreeplayMap(std::string newMap)
{
	_globalCvarManager->getCvar("freeplay_map").setValue(newMap);
}

void PremierSuite::setNewGUIKeybind(std::string newKeybind)
{
	_globalCvarManager->getCvar("ps_gui_keybind").setValue(newKeybind);
}

void PremierSuite::setNewPluginKeybind(std::string newKeybind)
{
	_globalCvarManager->getCvar("ps_toggle_keybind").setValue(newKeybind);
}

std::string PremierSuite::getClient()
{
	if (gameWrapper->IsUsingSteamVersion())
		return "Steam";
	else {
		return "Epic Games";
	}
}

// <summary>Return lines containing the given search string.</summary>
// <param name="searchString">Name of the string to search</param>
// <returns> Vector of std::string keybinds.</returns>

// <summary>Check correct keybinds are set from CFG onLoad()</summary>
void PremierSuite::handleKeybindCvar() {
#ifdef DEBUG
	std::vector<std::string> guiKeybinds = parseCfg("togglemenu PremierSuite", true);
	std::vector<std::string> pluginKeybinds = parseCfg("change_ps_enabled", true);
#else 
	std::vector<std::string> guiKeybinds = parseCfg("togglemenu PremierSuite", false);
	std::vector<std::string> pluginKeybinds = parseCfg("change_ps_enabled", false);
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
		cvarManager->getCvar("ps_toggle_keybind").setValue(pluginKeybinds[0]);
	}
	else {
		bool pluginbound = false;
		for (std::string bind : pluginKeybinds) {
			if (pluginbound == false) {
				if (bind == "Unset") { cvarManager->removeBind(bind); } //remove unset binding, shouldnt happen
				else {
					cvarManager->setBind(bind, "change_ps_enabled");
					cvarManager->getCvar("ps_toggle_keybind").setValue(bind);
					pluginbound = true;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
//--- Server Hooks
//-----------------------------------------------------------------------------

void PremierSuite::callbackSetDelay(ServerWrapper server, void* params, std::string eventName, std::function<void()> callback, bool queue) {
	float delay = 0;
	float delaySetting = 0;
	if (server.IsNull()) { return; }
	if (queue) { float delaySetting = *delayQueue; }
	else { float delaySetting = *delayExit; };
	if (*autoGG) { delay = delaySetting + *autoGGDelay; }
	else { delay = *delayExit; }
	if (isTournament(server)) {
		return;
	}
	if (!server.IsNull() && (server.GetPlaylist().memory_address != NULL) && (*disablePrivate || *disableExitCasual)) {
		if (isStandard(server) && *disableExitCasual) {
			LOG("Casual exit disabled: returning");
			return;
		}
		if (isPrivate(server) && *disablePrivate) {
			LOG("private disabled, returning");
			return;
		}
	}
	gameWrapper->SetTimeout([callback = std::move(callback)](...) {callback(); }, delaySetting);
}

void PremierSuite::executeQueue()
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

void PremierSuite::executeFreeplay()
{
	std::stringstream launchTrainingCommandBuilder;
	
	std::string mapname = *freeplayMap;
	std::string mapcode = GetKeyFromValue(mapname);

	if (mapname.compare("random") == 0)
	{
		mapcode = gameWrapper->GetRandomMap();
	}

	launchTrainingCommandBuilder << "start " << mapcode << "?Game=TAGame.GameInfo_Tutorial_TA?GameTags=Freeplay";
	LOG("command: {}", launchTrainingCommandBuilder.str());
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

void PremierSuite::executeCustomTraining()
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
	cvarManager->executeCommand("load_training " + *customCode);
}

void PremierSuite::executeWorkshop()
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

void PremierSuite::executeMainMenu()
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
	cvarManager->executeCommand("unreal_command disconnect");
}

void PremierSuite::onMatchEnd(ServerWrapper server, void* params, std::string eventName)
{
	if (server.IsNull()) { return; }

	if (*enableQueue) {
		callbackSetDelay(server, params, eventName, [this]() { this->executeQueue(); }, false);
	}
	if (*exitEnabled) {
		callbackSetDelay(server, params, eventName, [this]() { this->executeMainMenu(); }, false);
	}
	else {
		if (*freeplayEnabled) {
			callbackSetDelay(server, params, eventName, [this]() { this->executeFreeplay(); }, false);
		}
		else
			if (*customEnabled) {
				callbackSetDelay(server, params, eventName, [this]() { this->executeCustomTraining(); }, false);
			}
			else
				if (*workshopEnabled) {
					callbackSetDelay(server, params, eventName, [this]() { this->executeWorkshop(); }, false);
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
	BakkesModConfigFolder = gameWrapper->GetBakkesModPath() / L"cfg";
	std::filesystem::path PremierSuiteDataFolder = gameWrapper->GetDataFolder() / L"premiersuite";
	if (!exists(PremierSuiteDataFolder)) {
		std::filesystem::create_directory(PremierSuiteDataFolder);
	}

	RocketLeagueExecutableFolder = std::filesystem::current_path();

	// Initialize map codes one time and point to them
	std::vector<std::string> codes = GetFreeplayMapCodesStr();
	freeplayMapCodes = std::make_shared<std::vector<std::string>>(codes);

	registerCvars();
	registerNotifiers();

	gameWrapper->SetTimeout([this](GameWrapper* gw) { this->handleKeybindCvar(); }, 1);

	hookMatchEnded();
}

//std::vector<char*> PremierSuite::strlist(std::vector<std::string>& input) {
//	std::vector<char*> result;
//
//	// remember the nullptr terminator
//	result.reserve(input.size() + 1);
//
//	std::transform(begin(input), end(input),
//		std::back_inserter(result),
//		[](std::string& s) { return s.data(); }
//	);
//	result.push_back(nullptr);
//	return result;
//}

void PremierSuite::registerNotifiers() {

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
		std::vector<std::filesystem::path> workshops = getWorkshopMaps(WORKSHOP_MAPS_PATH);
		std::map<std::string, std::string> files = get_upk_files(WORKSHOP_MAPS_PATH.string());


		}, "", PERMISSION_ALL);

	cvarManager->registerNotifier("ps_get_map", [this](std::vector<std::string> args) {

		std::string map = getInGameMap();
			LOG("Current Map: {}", map);
		}, "", PERMISSION_ALL);

	cvarManager->registerNotifier("ps_evaluate", [this](std::vector<std::string> args) {
		LOG("BOOLS--------------------------");
			LOG("Plugin enabled: {}", PremierSuite::btos(cvarManager->getCvar("plugin_enabled").getBoolValue()));
			LOG(btos(*enabled));
			LOG("Freeplay enabled: {}", PremierSuite::btos(cvarManager->getCvar("freeplay_enabled").getBoolValue()));
			LOG(btos(*freeplayEnabled));
			LOG("CustomT enabled: {}", PremierSuite::btos(cvarManager->getCvar("custom_enabled").getBoolValue()));
			LOG(btos(*customEnabled));
			LOG("Exit enabled: {}", PremierSuite::btos(cvarManager->getCvar("exit_enabled").getBoolValue()));
			LOG(btos(*exitEnabled));
			LOG("Disable Private: {}", PremierSuite::btos(cvarManager->getCvar("disable_private").getBoolValue()));
			LOG(btos(*disablePrivate));
			LOG("Queue enabled: {}", PremierSuite::btos(cvarManager->getCvar("queue_enabled").getBoolValue()));
			LOG(btos(*enableQueue));
			LOG("Queue Disabled Casual: {}", PremierSuite::btos(cvarManager->getCvar("disable_queue_casual").getBoolValue()));
			LOG(btos(*disableQueueCasual));
			LOG("exit Disabled Casual: {}", PremierSuite::btos(cvarManager->getCvar("disable_exit_casual").getBoolValue()));
			LOG(btos(*disableExitCasual));
			LOG("STRINGS-------------------------");
			LOG("GUI Keybind: {}", cvarManager->getCvar("ps_gui_keybind").getStringValue());
			LOG(*gui_keybind);
			LOG("Plugin Keybind: {}", cvarManager->getCvar("ps_toggle_keybind").getStringValue());
			LOG(*plugin_keybind);
			LOG("Freeplay Map: {}", cvarManager->getCvar("freeplay_map").getStringValue());
			LOG(*freeplayMap);
			LOG("Custom Code: {}", cvarManager->getCvar("custom_code").getStringValue());
			LOG(*customCode);

		}, "", PERMISSION_ALL);

	cvarManager->registerNotifier("change_ps_enabled", [this](std::vector<std::string> args) {

		quickPluginEnabled();

		}, "", PERMISSION_ALL);


};

void PremierSuite::registerCvars() {

	//-----------------------------------------------------------------------------
	// Enable / Disable Feature (BOOL) --------------------------------------------
	//-----------------------------------------------------------------------------

	// whole plugin
	enabled = std::make_shared<bool>(true);
	cvarManager->registerCvar("plugin_enabled", "1", "Enable PremierSuite").bindTo(enabled);
	cvarManager->getCvar("plugin_enabled").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*enabled = cvar.getBoolValue();
		}
	);

	// queue
	enableQueue = std::make_shared<bool>(false);
	cvarManager->registerCvar("queue_enabled", "1", "Don't automatically queue when ending a casual game.").bindTo(enableQueue);
	cvarManager->getCvar("queue_enabled").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*enableQueue = cvar.getBoolValue();
		}
	);

	// freeplay
	freeplayEnabled = std::make_shared<bool>(false);
	cvarManager->registerCvar("freeplay_enabled", "0", "Enable Instant Exit -> Freeplay").bindTo(freeplayEnabled);
	cvarManager->getCvar("freeplay_enabled").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*freeplayEnabled = cvar.getBoolValue();

		}
	);

	// custom-training
	customEnabled = std::make_shared<bool>(false);
	cvarManager->registerCvar("custom_enabled", "0", "Enable Instant Exit -> Custom Training").bindTo(customEnabled);
	cvarManager->getCvar("custom_enabled").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*customEnabled = cvar.getBoolValue();

		}
	);

	// main-menu
	exitEnabled = std::make_shared<bool>(false);
	cvarManager->registerCvar("exit_enabled", "0", "Enable Instant Exit -> Main Menu").bindTo(exitEnabled);
	cvarManager->getCvar("exit_enabled").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*exitEnabled = cvar.getBoolValue();
		}
	);

	// workshop
	workshopEnabled = std::make_shared<bool>(false);
	cvarManager->registerCvar("workshop_enabled", "0", "Enable Instant Exit -> Main Menu").bindTo(workshopEnabled);
	cvarManager->getCvar("workshop_enabled").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*workshopEnabled = cvar.getBoolValue();
		}
	);

	// disable-private
	disablePrivate = std::make_shared<bool>(true);
	cvarManager->registerCvar("disable_private", "1", "Disable plugin during Private, Tournament, and Heatseeker matches.").bindTo(disablePrivate);
	cvarManager->getCvar("disable_private").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*disablePrivate = cvar.getBoolValue();
		}
	);

	// disable-exit-casual
	disableExitCasual = std::make_shared<bool>(false);
	cvarManager->registerCvar("disable_exit_casual", "1", "Don't automatically exit when ending a casual game.").bindTo(disableExitCasual);
	cvarManager->getCvar("disable_exit_casual").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*disableExitCasual = cvar.getBoolValue();
		}
	);

	// auto-gg
	autoGG = std::make_shared<bool>(false);
	cvarManager->registerCvar("ranked_autogg", "0", "Automatically say GG after the game.").bindTo(autoGG);
	cvarManager->getCvar("ranked_autogg").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*autoGG = cvar.getBoolValue();
		}
	);

	// disable-queue-casual
	disableQueueCasual = std::make_shared<bool>(false);
	cvarManager->registerCvar("disable_queue_casual", "0", "Don't automatically queue when ending a casual game.").bindTo(disableQueueCasual);
	cvarManager->getCvar("disable_queue_casual").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*disableQueueCasual = cvar.getBoolValue();
		}
	);

	//-----------------------------------------------------------------------------
	// Keybinds | Codes | Maps (STRING) -------------------------------------------
	//-----------------------------------------------------------------------------

	// gui keybind
	gui_keybind = std::make_shared<std::string>("F3");
	cvarManager->registerCvar("ps_gui_keybind", "F3", "Keybind to open the GUI").bindTo(gui_keybind);
	cvarManager->getCvar("ps_gui_keybind").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*gui_keybind = cvar.getStringValue();
		}
	);

	// plugin keybind
	plugin_keybind = std::make_shared<std::string>("Unset");
	cvarManager->registerCvar("ps_toggle_keybind", "Unset", "Enable/disable plugin Keybind").bindTo(plugin_keybind);
	cvarManager->getCvar("ps_toggle_keybind").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*plugin_keybind = cvar.getStringValue();
		}
	);

	// custom-training code
	customCode = std::make_shared<std::string>("A0FE-F860-967D-E628"); //TODO: check which sets default val
	cvarManager->registerCvar("custom_code", "A0FE-F860-967D-E628", "Custom-training code.").bindTo(customCode);
	cvarManager->getCvar("custom_code").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*customCode = cvar.getStringValue();
		}
	);

	// freeplay map
	freeplayMap = std::make_shared<std::string>("Beckwith Park (Midnight)");
	cvarManager->registerCvar("freeplay_map", "Beckwith Park (Midnight)", "Determines the map (code, not name) that will launch for training.").bindTo(freeplayMap);
	cvarManager->getCvar("freeplay_map").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*freeplayMap = cvar.getStringValue();
		}
	);

	// workshop map
	workshopMap = std::make_shared<std::string>();
	cvarManager->registerCvar("workshop_map", "", "Map to load into workshop.", true, true, 0, true, 1).bindTo(workshopMap);
	cvarManager->getCvar("workshop_map").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*workshopMap = cvar.getStringValue();
		}
	);

	// workshop directory path
	workshopMapDirPath = std::make_shared<std::string>();
	cvarManager->registerCvar("ps_workshop_path", WORKSHOP_MAPS_PATH.string(),
		"Default path for your workshop maps directory").bindTo(workshopMapDirPath);

	// custom maps directory path
	customMapDirPath = std::make_shared<std::string>();
	cvarManager->registerCvar("ps_custom_path", CUSTOM_MAPS_PATH.string(),
		"Default path for your custom maps directory").bindTo(customMapDirPath);

	//-----------------------------------------------------------------------------
	// Timeout Delays (FLOAT) -----------------------------------------------------
	//-----------------------------------------------------------------------------

	// exit delay
	delayExit = std::make_shared<float>(0);
	cvarManager->registerCvar("exit_delay", "0", "Seconds to wait before loading into training mode (0 to 10 seconds).", true, true, 0, true, 10).bindTo(delayExit);
	cvarManager->getCvar("exit_delay").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*delayExit = cvar.getFloatValue();
		}
	);

	// queue delay
	delayQueue = std::make_shared<float>(0);
	cvarManager->registerCvar("queue_delay", "0", "Seconds to wait before starting queue (0 to 10 seconds).", true, true, 0, true, 10).bindTo(delayQueue);
	cvarManager->getCvar("queue_delay").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*delayQueue = cvar.getFloatValue();
		}
	);

	// autoGG delay
	autoGGDelay = std::make_shared<float>(1);
	cvarManager->registerCvar("ranked_autogg_delay", "0", "Delay for GG after the game.", true, true, 0, true, 5).bindTo(autoGGDelay);
	cvarManager->getCvar("ranked_autogg_delay").addOnValueChanged([this](std::string oldValue, CVarWrapper cvar) {
		*autoGGDelay = cvar.getFloatValue();
		}
	);
}


