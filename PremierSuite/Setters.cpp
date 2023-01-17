#include "pch.h"
#include "PremierSuite.h"

void PremierSuite::setEnablePlugin(std::shared_ptr<bool> newBool)
{
	cvarManager->getCvar("plugin_enabled").setValue(*newBool);
}

//void PremierSuite::setEnableThemes(std::shared_ptr<bool> newBool)
//{
//	cvarManager->getCvar("themes_enabled").setValue(*newBool);
//}

void PremierSuite::setEnableFreeplay(std::shared_ptr<bool> newBool)
{
	cvarManager->getCvar("freeplay_enabled").setValue(*newBool);
	cvarManager->getCvar("custom_enabled").setValue(false);
	cvarManager->getCvar("workshop_enabled").setValue(false);
	cvarManager->getCvar("exit_enabled").setValue(false);
}

void PremierSuite::setEnableCustomTraining(std::shared_ptr<bool> newBool)
{
	cvarManager->getCvar("custom_enabled").setValue(*newBool);
	cvarManager->getCvar("freeplay_enabled").setValue(false);
	cvarManager->getCvar("workshop_enabled").setValue(false);
	cvarManager->getCvar("exit_enabled").setValue(false);
}

void PremierSuite::setEnableWorkshop(std::shared_ptr<bool> newBool)
{
	cvarManager->getCvar("workshop_enabled").setValue(*newBool);
	cvarManager->getCvar("freeplay_enabled").setValue(false);
	cvarManager->getCvar("custom_enabled").setValue(false);
	cvarManager->getCvar("exit_enabled").setValue(false);
}

void PremierSuite::setEnableExit(std::shared_ptr<bool> newBool)
{
	cvarManager->getCvar("exit_enabled").setValue(*newBool);
	cvarManager->getCvar("freeplay_enabled").setValue(false);
	cvarManager->getCvar("custom_enabled").setValue(false);
	cvarManager->getCvar("workshop_enabled").setValue(false);
}

void PremierSuite::setEnableQueue(std::shared_ptr<bool> newBool)
{
	cvarManager->getCvar("queue_enabled").setValue(*newBool);
}

void PremierSuite::setDisableQueueCasual(std::shared_ptr<bool> newBool)
{
	cvarManager->getCvar("disable_queue_casual").setValue(*newBool);
}

void PremierSuite::setDisableExitCasual(std::shared_ptr<bool> newBool)
{
	cvarManager->getCvar("disable_exit_casual").setValue(*newBool);
}

void PremierSuite::setDisablePrivate(std::shared_ptr<bool> newBool)
{
	cvarManager->getCvar("disable_private").setValue(*newBool);
}

void PremierSuite::setDelayQueue(std::shared_ptr<float>  newFloat)
{
	cvarManager->getCvar("queue_delay").setValue(*newFloat);
	LOG("Queue delay: {}.", std::to_string(*newFloat));
}

void PremierSuite::setDelayExit(std::shared_ptr<float> newFloat)
{
	cvarManager->getCvar("exit_delay").setValue(*newFloat);
}

void PremierSuite::setCustomTrainingCode(char newCode)
{
	cvarManager->getCvar("custom_code").setValue(newCode);
	*customEnabled = true;
	setEnableCustomTraining(customEnabled);

}

void PremierSuite::setCustomTrainingCode(std::string newCode)
{
	cvarManager->getCvar("custom_code").setValue(newCode);
	*customEnabled = true;
	setEnableCustomTraining(customEnabled);
}

void PremierSuite::setFreeplayMap(std::string newMap)
{
	cvarManager->getCvar("freeplay_map").setValue(newMap);
	*freeplayEnabled = true;
	setEnableFreeplay(freeplayEnabled);
}

//void PremierSuite::setTheme(std::string newTheme)
//{
//	cvarManager->getCvar("ps_current_theme").setValue(newTheme);
//}


void PremierSuite::setWorkshopMap(std::string newMap)
{
	cvarManager->getCvar("workshop_map").setValue(newMap);
	*workshopEnabled = true;
	setEnableWorkshop(workshopEnabled);
}

void PremierSuite::setNewGUIKeybind(std::string newKeybind)
{
	cvarManager->getCvar("ps_gui_keybind").setValue(newKeybind);
}

void PremierSuite::setNewPluginKeybind(std::string newKeybind)
{
	cvarManager->getCvar("ps_toggle_keybind").setValue(newKeybind);
}

