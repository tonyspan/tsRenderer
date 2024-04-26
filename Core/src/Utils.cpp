#include "Utils.h"

#include "Log.h"

#include "Buffer.h"
#include "FileStream.h"

#include <filesystem>

std::string GetProjectDirectory()
{
	auto currentPath = std::filesystem::current_path();
	NormalizePath(currentPath);

	return currentPath.string();
}

void NormalizePath(std::filesystem::path& path)
{
	path.make_preferred();
}

bool ReadFromFile(Buffer& buffer, const std::filesystem::path& path)
{
	if (path.empty())
		return false;

	FileStreamReader stream(path);

	if (!stream.IsStreamGood())
	{
		LOG("Failed to open %s", path.string().data());
		return false;
	}

	auto size = stream.GetFileSize();

	buffer.Allocate(size);

	stream.Read(buffer);

	return true;
}

bool WriteToFile(const std::filesystem::path& path, const Buffer& buffer)
{
	FileStreamWriter stream(path);

	if (!stream.IsStreamGood())
	{
		LOG("Failed to open %s", path.string().data());
		return false;
	}

	stream.Write(buffer);

	return true;
}

uint64_t HashString(const std::string& str)
{
	uint64_t hash = 5381;

	for (const auto& c : str)
		hash = 33 * hash + static_cast<uint64_t>(c);

	return hash;
}

void Delete(const std::filesystem::path& path, const std::string& what)
{
	if (std::filesystem::exists(path) && std::filesystem::is_directory(path))
	{
		for (auto const& entry : std::filesystem::directory_iterator(path))
		{
			const auto& extension = entry.path().extension();
			if (std::filesystem::is_regular_file(entry) && extension.string() == what)
				std::filesystem::remove(entry);
		}
	}
}
