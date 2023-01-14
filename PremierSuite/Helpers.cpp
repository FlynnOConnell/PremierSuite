#include "pch.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include "PremierSuite.h"

std::string PremierSuite::btos(bool x)
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

// call this once, onload
[[nodiscard]] std::vector<std::string> PremierSuite::ValsToVec(std::map<std::string, std::string> map) const
{
	auto kv = std::views::values(map);
	std::vector<std::string> keys{ kv.begin(), kv.end() };
	return keys;
}

// call this once, onload
[[nodiscard]] std::vector<std::string> PremierSuite::KeysToVec(std::map<std::string, std::string> map) const
{
	auto kv = std::views::keys(map);
	std::vector<std::string> keys{ kv.begin(), kv.end() };
	return keys;
}

int* PremierSuite::getIndex(std::vector<std::string> v, std::string str)
{
	auto it = std::find(v.begin(), v.end(), str);
	int idx;
	// If element was found
	if (it != v.end())
	{
		int idx = it - v.begin();
	}
	else { // not found

		int idx = -1;
	}
	return &idx;
}

std::string PremierSuite::GetKeyFromValue(std::map<std::string, std::string> map, std::string val)
{
	auto it = std::find_if(std::begin(map), std::end(map),
		[&val](const auto& p)
		{
			return p.second == val;
		}
	);

	if (it != std::end(map))
	{
		return it->first;
	}
	else {
		return it->first;
	}
}

std::vector<std::string> PremierSuite::parseCfg(std::filesystem::path filePath, const std::string searchString, bool log)
{
	const std::string bind = searchString;
	std::ifstream file(filePath);
	if (!exists(filePath)) return {};

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

void PremierSuite::logVector(std::vector<std::string> inputVec)
{
	for (std::vector<std::string>::iterator t = inputVec.begin(); t != inputVec.end(); ++t)
	{
		LOG("{}", *t);
	}
}

void PremierSuite::checkConflicts()
{
	cvarManager->executeCommand("plugin unload instantsuite");
	cvarManager->executeCommand("writeplugins");
}

[[nodiscard]] void PremierSuite::set_udk_files(const std::filesystem::path& root_dir) {
	if (!gameWrapper->IsUsingSteamVersion()) return;
	for (const auto& entry : std::filesystem::recursive_directory_iterator(root_dir)) {
		if (entry.is_directory()) {
			const std::filesystem::path dir_path = entry.path();
			for (const auto& upk_entry : std::filesystem::directory_iterator(dir_path)) {
				std::string mapName;
				std::string mapID;

				if (upk_entry.path().extension() == ".udk") {

					mapName = upk_entry.path().stem().string();
					mapID = dir_path.filename().string().c_str();
					LOG("{}, {}", mapName, mapID);
					WorkshopMaps.insert(std::pair<std::string, std::string>(mapName, mapID));
					break;
				}
			}
		}
	}
	return;
}

std::vector<std::string> PremierSuite::themesToVec()
{
	std::vector<std::string> filenames;
	std::filesystem::path themeFolder = PREMIERSUITE_DATA_PATH / "themes";

	for (const auto& entry : std::filesystem::recursive_directory_iterator(themeFolder)) {
		filenames.push_back(entry.path().filename().string());
	}
	return filenames;
}