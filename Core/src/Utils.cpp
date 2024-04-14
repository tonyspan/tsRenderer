#include "Utils.h"

#include "Log.h"

#include "Buffer.h"

#include <filesystem>
#include <fstream>

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

	const std::string& pathString = path.string();

	std::ifstream stream(pathString.data(), std::ios::binary | std::ios::ate);

	if (!stream.is_open())
	{
		LOG("Failed to open %s", pathString.data());
		return false;
	}

	std::streampos end = stream.tellg();
	stream.seekg(0, std::ios::beg);
	size_t size = end - stream.tellg();

	buffer.Allocate(size);

	if (size > 0)
	{
		stream.read(buffer.As<char>(), size);
		stream.close();
	}

	return true;
}

bool WriteToFile(const std::filesystem::path& path, const Buffer& buffer)
{
	const std::string& pathString = path.string();

	std::ofstream stream(pathString.data(), std::ios::out | std::ios::binary);

	if (!stream.is_open())
	{
		LOG("Failed to open %s", pathString.data());
		return false;
	}

	stream.write(buffer.As<char>(), (size_t)buffer.GetSize());
	stream.close();

	return true;
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
