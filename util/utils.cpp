#include "pch.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <Windows.h>
#include <filesystem>
#include "PremierSuite.h"
#include "GuiBase.h"

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
std::vector<std::filesystem::path> PremierSuite::getWorkshopMaps(const std::filesystem::path& workshopPath)
{
	if (!exists(workshopPath)) {
		return std::vector<std::filesystem::path>();
	}

	// Make sure we don't request workshop map names every tick.
	const bool shouldRequestWorkshopMapNames = publishedFileID.empty();
	std::vector<std::filesystem::path> files = IterateDirectory(workshopPath, {".udk"}, 0, 1);
	std::filesystem::path bestPath;
	std::vector<std::filesystem::path> workshopMaps;
	for (const std::filesystem::path& file : files) {
		if (file.parent_path() != bestPath.parent_path()) {
			if (!bestPath.empty()) {
				// this is the name of the folder which houses the workshop map
				const uint64_t workshopMapId = std::strtoull(bestPath.parent_path().stem().string().c_str(), nullptr,
					10);
				if (shouldRequestWorkshopMapNames && subscribedWorkshopMaps.find(workshopMapId) == subscribedWorkshopMaps.end()) {
					publishedFileID.push_back(workshopMapId);
				}
				workshopMaps.push_back(bestPath);
			}
			bestPath = file;
		}
	}

	if (!bestPath.empty()) {
		const uint64_t workshopMapId = std::strtoull(bestPath.parent_path().stem().string().c_str(), nullptr, 10);
		if (shouldRequestWorkshopMapNames && subscribedWorkshopMaps.find(workshopMapId) == subscribedWorkshopMaps.end()) {
			publishedFileID.push_back(workshopMapId);
		}
		//LOG("BestPath: {}", bestPath.string());
		workshopMaps.push_back(bestPath);
	}
	return workshopMaps;
}

// call this once, onload
[[nodiscard]] std::vector<std::string> PremierSuite::GetFreeplayMapCodesStr() const
{
	auto kv = std::views::values(FreeplayMaps);
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
		LOG("Index not found");
		int idx = -1;
	}
	return &idx;
}

std::string PremierSuite::GetKeyFromValue(std::string val)
{
	auto it = std::find_if(std::begin(FreeplayMaps), std::end(FreeplayMaps),
		[&val](const auto& p)
		{
			return p.second == val;
		}
	);

	if (it != std::end(maps))
	{
		return it->first;
	}
	else {
		return it->first;
	}
}

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

void PremierSuite::logVector(std::vector<std::string> inputVec)
{
	for (std::vector<std::string>::iterator t = inputVec.begin(); t != inputVec.end(); ++t)
	{
		LOG("{}", *t);
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

std::map<std::string, std::string> PremierSuite::get_upk_files(const std::string& root_dir) {
	std::map<std::string, std::string> upk_files;

	for (const auto& entry : std::filesystem::recursive_directory_iterator(root_dir)) {
		if (entry.is_directory()) {
			const auto dir_path = entry.path();
			for (const auto& upk_entry : std::filesystem::directory_iterator(dir_path)) {
				if (upk_entry.path().extension() == ".udk") {
					upk_files[upk_entry.path().string()] = dir_path.filename().string();
					break;
				}
			}
		}
	}
	return upk_files;
}