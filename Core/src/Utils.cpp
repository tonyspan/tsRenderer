#include "Utils.h"

#include <filesystem>
#include <algorithm>

std::string GetProjectDirectory()
{
	std::string currentPath = std::filesystem::current_path().string();
	NormalizePath(currentPath);

	return currentPath;
}

void NormalizePath(std::string& path)
{
#ifdef _WIN32
	std::replace(path.begin(), path.end(), '/', '\\');
#else
	std::replace(path.begin(), path.end(), '\\', '/');
#endif
}

void Delete(const std::string& path, const std::string& what)
{
	if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
	{
		for (auto const& entry : std::filesystem::directory_iterator(path))
		{
			const auto& extension = entry.path().extension();
			if (std::filesystem::is_regular_file(entry) && extension == what)
				std::filesystem::remove(entry);
		}
	}
}
